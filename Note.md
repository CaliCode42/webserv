# État actuel

## Terminé / testé

### Gestion des clients

Le `Server` écoute les connexions des `Clients` via des sockets non bloquantes et utilise `poll()` pour surveiller leurs événements.

> `poll()` — wait for some event on a file descriptor

Lorsqu'une nouvelle connexion arrive, le `Server` l'accepte et crée un `Client`, identifié par son file descriptor (`fd`).

Le `Server` peut ensuite surveiller les événements associés à chaque client.

Lorsqu'un client possède des données à lire :

```text
POLLIN
```

le `Server` lit les données reçues depuis la socket.

Une requête HTTP pouvant arriver en plusieurs morceaux, les données sont accumulées progressivement jusqu'à obtenir une `HttpRequest` complète.

Le traitement suit globalement ce chemin :

```text
Client socket
    │
    │ POLLIN
    ▼
Server
    │
    │ lecture des données
    ▼
Client / buffer de réception
    │
    ▼
HttpRequest construite progressivement
    │
    │ requête complète
    ▼
Handler::handle(HttpRequest)
    │
    ▼
HttpResponse
    │
    ▼
client.appendToWriteBuffer(response.serialize())
    │
    ▼
Server::enableClientWrite(fd)
    │
    │ POLLOUT
    ▼
Server::handleClientWrite(Client&)
```

Lorsque la socket devient disponible en écriture :

```text
POLLOUT
```

le `Server` envoie progressivement le contenu du buffer de réponse du `Client`.

Les lectures et écritures sont donc compatibles avec des opérations partielles et des sockets non bloquantes.

---

### Timeout d'inactivité des clients

Le `Server` surveille également l'inactivité des clients.

Si un client commence une requête mais cesse d'envoyer des données pendant une durée supérieure ou égale au timeout configuré, sa connexion est fermée.

Ce comportement a été testé notamment avec une requête HTTP volontairement incomplète.

Le serveur reste fonctionnel après la suppression du client inactif.

---

## Comportement actuel volontairement simplifié

### Fermeture du client après la réponse

Actuellement, lorsque le `Client` a terminé d'envoyer sa réponse, le `Server` le marque pour suppression :

```text
Server::handleClientWrite(Client&)
    │
    │ réponse entièrement envoyée
    ▼
Server::markClientForRemoval(fd)
    │
    ▼
fermeture de la connexion
```

Ce comportement est actuellement volontairement simplifié afin de faciliter le développement du serveur.

À terme, il faudra gérer correctement les connexions persistantes HTTP/1.1.

Le comportement cible sera plutôt :

```text
réponse entièrement envoyée
        │
        ▼
la connexion doit-elle être fermée ?
        │
     ┌──┴──┐
     │     │
    oui   non
     │     │
     ▼     ▼
 fermeture reset de l'état du Client
           │
           ▼
     attente d'une nouvelle requête
```

Cette partie reste donc à reprendre plus tard.

---

# CGI

## Configuration CGI

Le projet possède maintenant une première gestion de la configuration nécessaire aux CGI.

Une location peut définir :

- son URI ;
- son `root` ;
- les extensions CGI autorisées ;
- l'interpréteur associé à chaque extension.

Exemple :

```conf
location /cgi {
    root www/cgi-bin;

    cgi .py /usr/bin/python3;
    cgi .php /usr/bin/php-cgi;
}
```

Il faut distinguer trois informations :

```text
/cgi
│
└── URI de la location

www/cgi-bin
│
└── emplacement filesystem des scripts

.py  -> /usr/bin/python3
.php -> /usr/bin/php-cgi
│
└── association extension / interpréteur
```

Les associations CGI sont stockées dans une `std::map<std::string, std::string>`.

Le parsing suit globalement :

```text
ConfigParser::parse(file)
        │
        ▼
ConfigParser::parseLocation(...)
        │
        ├── path
        ├── root
        │
        └── cgi extension interpreter
                │
                ▼
        LocationConfig
                │
                ├── path
                ├── root
                └── map<extension, interpreter>
                │
                ▼
        ServerConfig::_locations
```

`ServerConfig` permet ensuite de retrouver la location correspondant à une URI grâce à `findLocation()`.

La résolution respecte le principe de la location correspondante la plus spécifique.

---

## Détection et résolution CGI

`CgiHandler` contient actuellement les helpers permettant de déterminer si une requête doit être traitée comme un CGI.

Le traitement permet notamment de faire :

```text
URI
 │
 ▼
ServerConfig::findLocation()
 │
 ▼
LocationConfig
 │
 ▼
CgiHandler::isCgiRequest()
 │
 ▼
extension du script
 │
 ▼
interpréteur configuré
```

Par exemple :

```text
/cgi/test.py
    │
    ▼
location /cgi
    │
    ▼
extension .py
    │
    ▼
/usr/bin/python3
```

Les URI contenant une query string sont également prises en charge :

```text
/cgi/test.py?name=Bob
        │
        ├── script URI : /cgi/test.py
        └── query      : name=Bob
```

Le chemin filesystem du script peut ensuite être construit à partir de la location :

```text
location : /cgi
root     : www/cgi-bin
URI      : /cgi/test.py?name=Bob

        ↓

script path : www/cgi-bin/test.py
query       : name=Bob
```

---

## Environnement CGI

`CgiHandler` peut construire les variables d'environnement nécessaires à l'exécution d'un CGI à partir de la `HttpRequest`.

Une `envMap` est utilisée pour stocker ces informations.

Les variables actuellement préparées comprennent notamment :

```text
REQUEST_METHOD
QUERY_STRING
CONTENT_LENGTH
CONTENT_TYPE
SCRIPT_FILENAME
SCRIPT_NAME
SERVER_PROTOCOL
```

Exemple pour :

```http
GET /cgi/test.py?name=Bob HTTP/1.1
```

on obtient notamment :

```text
REQUEST_METHOD=GET
QUERY_STRING=name=Bob
SCRIPT_FILENAME=www/cgi-bin/test.py
SCRIPT_NAME=/cgi/test.py
SERVER_PROTOCOL=HTTP/1.1
```

Les environnements GET et POST ont été testés indépendamment.

---

# CgiProcess

## État actuel

Une classe `CgiProcess` a été ajoutée pour représenter l'exécution d'un programme CGI.

Son rôle est distinct de celui de `CgiHandler`.

```text
CgiHandler
│
├── détecte les CGI
├── choisit l'interpréteur
├── résout le script
├── extrait la query string
└── construit l'environnement CGI


CgiProcess
│
├── crée le processus enfant
├── gère les pipes
├── exécute l'interpréteur
├── communique avec le CGI
└── surveille son cycle de vie
```

---

## Lancement d'un CGI

`CgiProcess::start()` met actuellement en place :

```text
CgiProcess::start()
        │
        ├── pipe stdin
        ├── pipe stdout
        │
        ▼
      fork()
     /      \
    /        \
 parent      child
   │           │
   │           ├── dup2(stdin)
   │           ├── dup2(stdout)
   │           │
   │           ▼
   │        execve()
   │
   ├── conserve stdin CGI
   ├── conserve stdout CGI
   └── FDs non bloquants
```

L'environnement stocké dans `envMap` est converti en `envp` compatible avec `execve()`.

L'enfant exécute l'interpréteur avec le script correspondant.

Par exemple :

```text
/usr/bin/python3 www/cgi-bin/test.py
```

---

## Communication avec le CGI

Deux pipes permettent la communication :

```text
Webserv
   │
   │ body HTTP
   ▼
CGI stdin
```

et :

```text
CGI stdout
   │
   ▼
Webserv
```

Les extrémités conservées par le parent sont non bloquantes.

Lorsque le body de la requête est vide, le stdin du CGI est immédiatement fermé afin que le processus reçoive EOF.

---

## Lecture de stdout

La sortie du CGI peut maintenant être lue progressivement grâce à `readOutput()`.

La lecture gère :

```text
read() > 0
→ ajout dans _output

read() == 0
→ EOF
→ fermeture stdout CGI

EAGAIN / EWOULDBLOCK
→ aucune donnée disponible actuellement
→ pas une erreur fatale

EINTR
→ opération interrompue
→ possibilité de réessayer
```

La sortie d'un vrai script Python a été récupérée avec succès :

```text
Content-Type: text/plain

Hello from CGI
```

---

## Cycle de vie du processus

La terminaison du processus enfant est surveillée avec :

```cpp
waitpid(pid, &status, WNOHANG);
```

On distingue :

```text
processus enfant terminé
```

de :

```text
CGI complètement terminé
```

Un CGI est considéré comme complètement terminé lorsque :

```text
_processExited == true
        &&
_stdoutClosed == true
```

Cela permet de continuer à lire les données encore présentes dans le pipe même si le processus enfant a déjà terminé.

Les processus enfants sont récupérés correctement afin d'éviter les zombies.

Les erreurs rencontrées pendant `start()` disposent également d'un nettoyage des pipes, FDs et processus déjà créés.

---

# Fonctionnalités CGI actuellement validées

Les éléments suivants sont implémentés et testés en isolation :

```text
Configuration
✓ parsing des directives CGI
✓ stockage extension -> interpréteur
✓ recherche de LocationConfig avec findLocation()

Résolution CGI
✓ détection d'une requête CGI
✓ choix de l'interpréteur
✓ résolution du chemin filesystem du script
✓ extraction de la query string

Environnement
✓ construction de l'envMap
✓ environnement GET
✓ environnement POST

Processus CGI
✓ création des pipes
✓ fork()
✓ dup2()
✓ execve()
✓ FDs parent non bloquants
✓ fermeture immédiate de stdin pour un body vide
✓ lecture non bloquante de stdout
✓ récupération de la sortie CGI
✓ waitpid(..., WNOHANG)
✓ détection de la fin du processus
✓ récupération correcte de l'enfant
✓ nettoyage des erreurs de démarrage

Test réel
✓ exécution d'un script Python
✓ récupération de sa sortie
✓ fin propre du processus
```

---

# En cours

Le moteur CGI fonctionne maintenant en isolation pour un CGI GET simple.

Il n'est cependant **pas encore intégré à la boucle `poll()` du `Server`**.

La prochaine étape est l'écriture du body HTTP vers le stdin du CGI.

Il faut implémenter une méthode du type :

```cpp
CgiProcess::writeInput()
```

qui devra gérer les écritures non bloquantes et partielles :

```text
body HTTP
    │
    ▼
_input
    │
    │ write()
    ▼
_stdinFd
    │
    ▼
CGI stdin
```

L'avancement dans le body sera suivi avec :

```text
_inputOffset
```

Lorsque tout le body aura été transmis :

```text
_inputOffset == _input.size()
        │
        ▼
close(_stdinFd)
        │
        ▼
CGI reçoit EOF
```

Cela permettra ensuite de tester correctement un CGI avec une requête POST.

---

# Prochaines étapes

Ordre prévu :

```text
1. Implémenter writeInput()
        │
        ▼
2. Tester l'écriture non bloquante du body
        │
        ▼
3. Tester un CGI POST complet
        │
        ▼
4. Intégrer les FDs CGI dans Server::poll()
        │
        ▼
5. Relier HttpRequest → CgiProcess
        │
        ▼
6. Gérer les timeouts et erreurs CGI
        │
        ▼
7. Parser la sortie CGI
        │
        ▼
8. Construire la HttpResponse correspondante
        │
        ▼
9. Envoyer la réponse au Client
```

L'architecture cible pour une requête CGI devient donc :

```text
Client
  │
  ▼
Server::poll()
  │
  ▼
HttpRequest
  │
  ▼
findLocation()
  │
  ▼
CgiHandler
  │
  ├── détecte CGI
  ├── résout interpreter
  ├── résout script
  └── construit envMap
  │
  ▼
CgiProcess
  │
  ├── fork / execve
  ├── stdin CGI
  ├── stdout CGI
  └── waitpid
  │
  ▼
sortie CGI
  │
  ▼
HttpResponse
  │
  ▼
Client
```

Le point de reprise actuel est donc :

> **Implémenter l'écriture non bloquante du body de la requête vers le stdin du CGI (`CgiProcess::writeInput()`), puis valider l'exécution d'un CGI POST avant de commencer l'intégration dans `Server::poll()`.**