# webserv

Un petit serveur HTTP écrit en C++ (conforme à la norme C++98) utilisé pour apprendre et expérimenter les concepts réseau, le parsing HTTP et la gestion basique de requêtes/réponses.

## Structure

- `Makefile`
- `srcs/` — code source (.cpp).
- `includes/` — headers (.hpp).
- `obj/` — fichiers objet générés.
- `www/` — fichiers statiques (ex : `index.html`, `about.html`).

## Architecture actuelle

### Server
```
├── gère les sockets
├── poll()
├── accept()
├── recv()
├── send()
└── possède les Clients
```

### Client
```
├── fd
├── buffer TCP
├── appendToBuffer()
├── hasCompleteRequest()
└── extractRequest()
```

### HttpRequest
```
├── parse()
├── method
├── path
├── version
└── headers
```

### RequestHandler
```
├── handle()
└── handleGet()
```

### HttpResponse
```
├── status
├── headers
├── body
└── toString()
```

## Contenu des dossiers (résumé)
-
- `srcs/`
	- `main.cpp` — point d'entrée, initialisation du serveur et boucle principale.
	- `Server.cpp` — implémentation de la classe `Server` (écoute, accept, dispatch).
	- `Client.cpp` — gestion d'un client connecté (socket, lecture/écriture non-bloquante si implémentée).
	- `HttpRequest.cpp` — parsing des requêtes HTTP entrantes (très basique pour l'instant, juste pour
	pouvoir implémenter le reste et uniquement pour GET)
	- `HttpResponse.cpp` — construction des réponses HTTP (uniquement pour GET pour l'instant)
	- `RequestHandler.cpp` — logique pour choisir/produire la réponse à partir d'une requête.
	- `ServerConfig.cpp` — lecture/stockage de la configuration du serveur.
- `includes/`
	- `Server.hpp`, `Client.hpp`, `HttpRequest.hpp`, `HttpResponse.hpp`, `RequestHandler.hpp`, `ServerConfig.hpp`, `Utils.hpp`

## Compilation
-
Pour compiler le projet :

```bash
make
```

L'exécutable produit est `webserv`.

Lancer le serveur (exemple) :

```bash
./webserv
```

## Description détaillée des classes
-
Chaque description indique le rôle principal, les responsabilités et les interactions clés avec les autres composants.

- `Server` (`includes/Server.hpp`, `srcs/Server.cpp`)
	- Rôle : point central du serveur. Crée la socket d'écoute, accepte les connexions entrantes, et orchestre la boucle d'événements (poll).
	- Responsabilités :
		- Ouvrir et configurer la socket d'écoute (bind, listen).
		- Accepter nouvelles connexions et instancier des objets `Client`.
		- Maintenir la liste des clients actifs et déléguer la lecture/écriture à `Client`.
		- Utiliser `ServerConfig` pour les paramètres (ports, root des documents, etc...).
	- Interactions : crée `Client`, appelle `RequestHandler` pour traiter les requêtes reçues.

- `Client` (`includes/Client.hpp`, `srcs/Client.cpp`)
	- Rôle : encapsule une connexion réseau entrante (socket) et l'état de parsing/écriture associé.
	- Responsabilités :
		- Lire depuis la socket et accumuler les données brutes.
		- Appeler le parser (`HttpRequest`) pour construire un objet requête quand assez de données sont reçues
		(requête complète)
		- Représente une connexion TCP et son buffer.
		- Gérer l'état de connexion (keep-alive, fermeture propre).
	- Interactions : interragit avec `Server` (gestion de la socket) et `RequestHandler` (pour obtenir des réponses).

- `HttpRequest` (`includes/HttpRequest.hpp`, `srcs/HttpRequest.cpp`)
	- Rôle : représenter et parser une requête HTTP entrante.
	- Responsabilités :
		- Extraire la méthode, l'URI, la version HTTP, les headers et le corps.
		- Fournir une API simple pour accéder aux headers, query string, et body.
	- Interactions : utilisé par `Client` pour convertir les octets reçus en structure logique exploitable par `RequestHandler`.

- `HttpResponse` (`includes/HttpResponse.hpp`, `srcs/HttpResponse.cpp`)
	- Rôle : construire la réponse HTTP à envoyer au client.
	- Responsabilités :
		- Définir le status code, status message, headers et body.
		- Sérialiser la réponse au format HTTP/1.1 prêt à l'envoi (incluant la gestion du `Content-Length`).
	- Interactions : créé par `RequestHandler` et envoyé par `Client`.

- `RequestHandler` (`includes/RequestHandler.hpp`, `srcs/RequestHandler.cpp`)
	- Rôle : logique métier pour traiter une `HttpRequest` et produire une `HttpResponse`.
	- Responsabilités :
		- Analyser l'URI et décider s'il s'agit d'un fichier statique (servir depuis `www/`) ou d'une route dynamique.
		- Construire les headers et le body de la réponse (lecture de fichiers, génération d'erreurs 4xx/5xx, redirections, etc.).
		- Respecter les directives provenant de `ServerConfig` (root, index, error pages, etc.).
	- Interactions : consulte le système de fichiers pour servir des fichiers statiques et utilise `HttpResponse` pour renvoyer le résultat.

- `ServerConfig` (`includes/ServerConfig.hpp`, `srcs/ServerConfig.cpp`)
	- Rôle : centraliser la configuration du serveur (ports, roots, virtual hosts, limites).
	- Responsabilités :
		- Parser un fichier de configuration (si présent) ou exposer des valeurs par défaut.
		- Fournir une API consultable par `Server` et `RequestHandler`.
	- Interactions : lu au démarrage par `main`/`Server` et consulté lors du traitement des requêtes.
	!!! En cours d'implémentation !!!

- `Utils` (`includes/Utils.hpp`)
	- Rôle : fonctions utilitaires partagées (parsing auxiliaire, encodages, helpers de chemin, gestion des erreurs).
	- Responsabilités : fournir des helpers réutilisables pour éviter la duplication.
	(actuellement juste turnIntoString)

## Flux d'une requête
```
Client
    │
    ▼
recv()
    │
    ▼
Client::buffer
    │
    ▼
HttpRequest::parse()
    │
    ▼
RequestHandler::handle()
    │
    ▼
HttpResponse
    │
    ▼
HttpResponse::toString()
    │
    ▼
send()
```

## Etat actuel et évolutions à venir
```
✔ Réseau (poll, accept, recv, send)
✔ Buffer TCP
✔ Parsing HTTP (basique pour GET)
⏳ GET statique (en cours)
⬜ Configuration
⬜ POST
⬜ CGI
⬜ DELETE
```

## Ressources


- [Poll](https://man7.org/linux/man-pages/man2/poll.2.html)

- [C++ Documentation](https://en.cppreference.com/)

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

- [MDN HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)
(https://www.studyplan.dev/pro-cpp/http)


- [Linux man pages](https://man7.org/linux/man-pages/)
