# Fiche commandes — soutenance webserv

> Une commande par question, prête à taper. `curl -i` = affiche status + headers + body en un seul appel, c'est la commande à retenir en priorité si tu ne dois en garder qu'une.
> Remplace `8080` par le port réel de ton serveur au moment du test.

---

## Démarrer le serveur

```bash
./webserv webserv.conf
```

---

## Q13 — Plusieurs ports différents

```bash
curl -i http://localhost:8080/
curl -i http://localhost:8081/
```
→ Compare le contenu retourné : différent selon le port si les `root` diffèrent.

## Q15 — Page d'erreur personnalisée (404)

```bash
curl -i http://localhost:8080/page-qui-nexiste-pas
```
→ Regarde le body de la réponse : doit être ta page `404.html` custom, pas juste `<h1>Not Found</h1>` générique.

## Q16 — Limite de taille du corps

```bash
curl -i -X POST --data "contenu court" http://localhost:8080/uploads/test.txt


curl -i -X POST --data "$(head -c 999999999 </dev/zero)" http://localhost:8080/uploads/test.txt
OU
head -c 999999999 /dev/zero | curl -i -X POST --data-binary @- http://localhost:8080/uploads/test.txt


Voici les cinq tests qui couvrent toutes les frontières de taille, à lancer un par un sur ton Mac (serveur déjà démarré avec `webserv.conf`, limite = `1048576`) :

**1. Content-Length — pile à la limite (1048576) → attendu 201**
```bash
head -c 1048576 /dev/zero | curl -s -i -X POST --data-binary @- http://localhost:8080/uploads/cl_exact.txt
```

**2. Content-Length — limite + 1 (1048577) → attendu 413**
```bash
head -c 1048577 /dev/zero | curl -s -i -X POST --data-binary @- http://localhost:8080/uploads/cl_over.txt
```

**3. Chunked — un seul chunk pile à la limite (taille hex `100000` = 1048576) → attendu 201**
```bash
{
  printf 'POST /uploads/chunk_exact.txt HTTP/1.1\r\n'
  printf 'Host: localhost\r\n'
  printf 'Transfer-Encoding: chunked\r\n'
  printf '\r\n'
  printf '100000\r\n'
  head -c 1048576 /dev/zero
  printf '\r\n'
  printf '0\r\n\r\n'
} | nc -w 3 localhost 8080
```

**4. Chunked — un seul chunk limite + 1 (taille hex `100001` = 1048577) → attendu 413**
```bash
{
  printf 'POST /uploads/chunk_over.txt HTTP/1.1\r\n'
  printf 'Host: localhost\r\n'
  printf 'Transfer-Encoding: chunked\r\n'
  printf '\r\n'
  printf '100001\r\n'
  head -c 1048577 /dev/zero
  printf '\r\n'
  printf '0\r\n\r\n'
} | nc -w 3 localhost 8080
```

**5. Chunked — taille annoncée déjà trop grande, sans même envoyer les données → attendu 413 immédiat**
```bash
{
  printf 'POST /uploads/chunk_announced_over.txt HTTP/1.1\r\n'
  printf 'Host: localhost\r\n'
  printf 'Transfer-Encoding: chunked\r\n'
  printf '\r\n'
  printf '100001\r\n'
} | nc -w 3 localhost 8080
``` sinon q1 ou q3
ce dernier test = le serveur rejette dès la ligne de taille du chunk, sans attendre une seule donnée — exactement la vérification `_chunkSize > _maxBodySize - _body.size()` qu'on vient de voir dans `parseChunkSize()`.

```
→ Premier : succès normal. Deuxième (dépasse `client_max_body_size`) : /413 Payload Too Large**.

## Q19 — Méthode non autorisée

```bash
curl -i -X DELETE http://localhost:8080/readonly/index.html

```
→ /405 Method Not Allowed**.

## Q20-22 — GET / POST / DELETE

```bash
curl -i http://localhost:8080/index.html
curl -i -X POST --data "hello" http://localhost:8080/uploads/hello.txt
curl -i -X DELETE http://localhost:8080/uploads/hello.txt

 verif bien suup
curl -i http://localhost:8080/uploads/hello.txt
```
→ 200 / 201 / 204. et /404

## Q21 — Méthode inconnue (ne doit pas crasher)

```bash
curl -i -X FOOBAR http://localhost:8080/
```
→ /501 Not Implemented**, serveur toujours vivant après (relance un `curl` normal juste après pour le prouver).
```bash
curl -i http://localhost:8080/
```


## Q23 — Upload puis récupération

```bash
curl -i -X POST --data "contenu du fichier" http://localhost:8080/uploads/monfichier.txt
curl -i http://localhost:8080/uploads/monfichier.txt
```
→ 201 puis 200 avec le contenu identique.

## Q28 — Onglet réseau du navigateur

*(pas de commande — ouvrir les outils de développement du navigateur, onglet Réseau/Network, avant de charger une page)*

## Q30 — Mauvaise URL

```bash
curl -i http://localhost:8080/nimportequoi
```
→ /404 Not Found**.

## Q31 — Répertoire sans autoindex ni index

```bash
curl -i http://localhost:8080/no-listing/
```
403, c'est quand la ressource existe mais que l'accès est refusé ; 
404(NOT FOUND), c'est quand elle n'existe vraiment pas."
→ /403 Forbidden**.

## Q32 — Autoindex (listing de répertoire)

```bash
curl -i http://localhost:8080/listing/
```
→ 200, body HTML avec la liste des fichiers du dossier.

## Q33 — URL redirigée

```bash
curl -i http://localhost:8080/old
```
→ /301** (ou 302 selon config) + header `Location: ...`. Ajoute `-L` pour suivre automatiquement la redirection : `curl -iL http://localhost:8080/old`.


echo "=== SANS gestion de cookies (session_id change entre les 2 sauts) ==="
curl -iL http://localhost:8080/old

echo ""
echo "=== AVEC gestion de cookies (session_id reste identique) ==="
rm -f cookies.txt
curl -iL -c cookies.txt -b cookies.txt http://localhost:8080/old




## Q34 — Plusieurs ports, sites différents

*(identique à Q13, comparer visuellement dans le navigateur sur les deux ports)*

## Q35 — Même port, même fichier de config (2 blocs `server`)

```bash
./webserv /tmp/dup_port.conf
```

# Vérifier qu'aucun processus n'écoute déjà sur 8080
lsof -i :8080

# Si quelque chose apparaît,Ctrl+C dans terminal, ou :
# kill <PID>

# Puis lance dup_port.conf seul
./webserv /tmp/dup_port.conf

cat /tmp/dup_port.conf

cat > /tmp/dup_port_named.conf << 'EOF'
server {
    listen 8080;
    server_name site1.local;
    root www;

    location / {
        root www;
        allow_methods GET;
    }
}

server {
    listen 8080;
    server_name site2.local;
    root www2;

    location / {
        root www2;
        allow_methods GET;
    }
}
EOF

mkdir -p www2
echo "<h1>SITE 2</h1>" > www2/index.html

lsof -i :8080   # vérifier que rien d'autre n'écoute déjà dessus
./webserv /tmp/dup_port_named.conf &
sleep 1

echo "=== Requête avec Host: site1.local ==="
curl -s http://localhost:8080/ -H "Host: site1.local"

echo ""
echo "=== Requête avec Host: site2.local ==="
curl -s http://localhost:8080/ -H "Host: site2.local"

kill %1

→ Démarre normalement, pas d'erreur (virtual hosting).

## Q36 — Même port, deux processus séparés

Terminal 1 :
```bash
./webserv webserv.conf
```
Terminal 2 (config différente, même port) :
```bash
./webserv /tmp/dup_port.conf
```
→ Terminal 2 : `Error: failed to bind socket.`, le processus s'arrête.

## Q37-38 — Stress test Siege

```bash
siege -b http://localhost:8080/
```
→ Laisser tourner puis `Ctrl+C` : regarder la ligne **Availability** (doit être ≥ 99.5%).

## Q39 — Pas de fuite mémoire

Avant le siege :
```bash
ps aux | grep webserv
```
Après le siege, relancer la même commande et comparer la colonne RSS (mémoire) :
```bash
ps aux | grep webserv
```
→ Valeur RSS stable = pas de fuite.

## Q40 — Pas de connexion accrochée (timeout)

```bash
nc localhost 8080
```
→ Ne rien taper, attendre >10s : la connexion se ferme d'elle-même. Pendant l'attente, ouvrir un second terminal et vérifier que le serveur répond toujours normalement :
```bash
curl -i http://localhost:8080/
```

## Q41 — Utilisable indéfiniment

*(découle de Q39 + Q40, pas de commande propre — relancer `siege -b` une seconde fois pour montrer que rien n'a dégradé)*

## Q42 — Cookies / sessions

Première visite (crée une session) :
```bash
curl -i -c cookies.txt http://localhost:8080/
```
→ Regarder le header `Set-Cookie` dans la réponse.

Visite suivante avec le même cookie (réutilise la session) :
```bash
curl -i -b cookies.txt http://localhost:8080/
```
→ Pas de nouveau `Set-Cookie` dans la réponse (session déjà connue).

**Test complémentaire — prouver que c'est bien le cookie qui décide (pas l'IP) :**
```bash
curl -i -c cookies_A.txt http://localhost:8080/   # visiteur A, 1ère fois
curl -i -b cookies_A.txt http://localhost:8080/   # visiteur A revient (même cookie)

si curl i seul, cookies non conservé
curl -i http://localhost:8080/                     # sans cookie du tout
curl -i http://localhost:8080/                     # sans cookie du tout, encore
```
→ Attendu : 1er appel donne un `Set-Cookie` ; 2e appel n'en donne pas (reconnu) ; 3e et 4e appels donnent chacun un `Set-Cookie` avec un `session_id` **différent** l'un de l'autre — même s'ils viennent de la même machine. Ça montre que sans cookie valide, chaque requête est traitée comme un nouveau visiteur, et que les IDs générés sont bien aléatoires (lien direct avec `srand()` dans `main.cpp`).

Phrase prête si on te pousse dessus : *"le serveur ne se base que sur le header Cookie de la requête pour reconnaître un visiteur — sans cookie valide, il considère systématiquement que c'est quelqu'un de nouveau."*


CREE AVEC 200 ET PAS 201
Dans MethodHandler::handle() (lignes 62-83, 137-140) : la vérification/création de session se fait tout au début, avant même de savoir si la requête va aboutir sur un 200, 201, 404 ou autre. req.getCookies() est lu, une nouvelle session créée si besoin, et le Set-Cookie est ajouté à la toute fin, juste avant de renvoyer response — quel que soit le code de statut décidé entre-temps par handleGet(), handlePost() ou handleDelete().
Autrement dit, la logique de session est complètement indépendante de la logique de méthode/statut — chaque requête qui arrive, GET ou POST, réussie ou en erreur, passe par cette vérification de cookie en premier. C'est cohérent avec ce que tu as observé : même tes 404 et 403 ont un Set-Cookie, parce que reconnaître le visiteur n'a rien à voir avec si le fichier demandé existe.



**Supprimer une session manuellement (`destroy()`) :**

`SessionManager` n'expose pas de route HTTP dédiée pour supprimer une session à la demande (pas de logout construit dans ce projet) — `destroy()` est une méthode C++ interne, appelée côté serveur. Deux façons de le montrer en défense :

1. *Illustrer que la méthode existe dans le code* : montrer `SessionManager::destroy()` dans `SessionManager.cpp`, expliquer qu'elle fait `_sessions.erase(sid)` et qu'elle serait typiquement appelée sur une route `/logout` si le projet en avait une.

2. *Test observable équivalent — laisser expirer la session (TTL)* : sans attendre le TTL complet, on peut simuler l'effet en supprimant le cookie côté client pour prouver que le serveur ne reconnaît plus rien sans lui :
```bash
curl -i -c cookies.txt http://localhost:8080/    # crée la session, Set-Cookie présent
rm cookies.txt                                    # "perte" du cookie côté client
curl -i http://localhost:8080/                    # nouvelle requête sans cookie
```
→ Le 2e appel obtient un **nouveau** `session_id` différent du premier : du point de vue du serveur, c'est comme si l'ancienne session n'existait plus pour ce client (même si elle reste techniquement en mémoire jusqu'à expiration du TTL, faute de `destroy()` appelée).

Phrase prête : *"il n'y a pas de suppression déclenchée par une requête HTTP dans notre projet, seulement `destroy()` en interne côté C++ et l'expiration automatique par TTL — mais on peut simuler l'effet en ne renvoyant plus le cookie."*

Deux niveaux possibles, selon ce que tu veux vraiment supprimer :

**1. Supprimer le cookie côté client (le plus simple, refait à chaque test)**
```bash
rm cookies.txt
```
Le fichier disparaît, donc ton prochain `curl -b cookies.txt` n'aura rien à envoyer — le serveur ne reconnaîtra plus rien et créera une nouvelle session à la prochaine requête. C'est l'équivalent de "l'utilisateur efface son cookie navigateur."

**2. Supprimer la session côté serveur**
Ça, en revanche, ton projet ne le permet pas via une requête HTTP — il n'y a pas de route `/logout` ni de méthode DELETE exposée pour ça. `SessionManager::destroy()` existe bien dans le code C++ (elle fait `_sessions.erase(sid)`), mais rien ne l'appelle actuellement dans `MethodHandler`. Donc côté serveur, la session continue d'exister en mémoire jusqu'à expiration naturelle par TTL, même si tu supprimes le cookie localement.

Si un évaluateur te demande "comment on supprime une session manuellement", la réponse honnête est : *"côté client, on peut supprimer le cookie pour que le serveur ne nous reconnaisse plus ; côté serveur, `destroy()` existe dans `SessionManager` mais n'est appelée par aucune route dans ce projet — il n'y a pas de logout implémenté."*

---

AUTRE

curl -i http://localhost:8080/test.png
si fail
Voici comment le présenter, si un évaluateur regarde tes scripts de tests automatisés et voit des `[FAIL]` :

**La structure de la réponse, en trois temps :**

1. **Assume-le sans te justifier à l'excès** : *"J'ai des scripts de tests automatisés qui couvrent beaucoup de cas, et certains échouent — mais j'ai vérifié chacun individuellement, et aucun n'est un vrai bug de mon code."*

2. **Explique les catégories, brièvement** :
   - *"Certains tests ciblent des routes qui n'existent que dans un fichier de config de test différent (`test.conf`), pas dans `webserv.conf` que j'utilise ce soir — donc ces routes n'existent tout simplement pas dans ma config actuelle."*
   - *"D'autres utilisent des options d'outils Unix qui ne sont pas disponibles sur macOS (`nc -q`, `timeout`) — c'est une limite de mon environnement de test, pas de mon serveur."*
   - *"Un test attendait un `404` là où mon serveur renvoie `403`, ce qui est en fait le code HTTP plus correct pour un dossier sans page d'accueil — c'est le test qui avait la mauvaise attente, pas mon code."*

3. **Termine sur ce qui compte** : *"Chaque cas que j'ai vérifié manuellement en live — les frontières de taille, les codes d'erreur, les sessions, les redirections — fonctionne correctement."*

**Pourquoi cette approche est solide :** ça montre exactement ce qu'un évaluateur 42 veut voir — pas "tout est parfait", mais une vraie capacité à **diagnostiquer** pourquoi un test échoue avant de conclure que le code est cassé. Trouver et corriger les deux vrais bugs ce soir (414, 431) pendant qu'on testait, puis distinguer ça des faux positifs sur les tests automatisés — c'est exactement la démarche rigoureuse qu'ils cherchent.



Voici les cinq commandes, une par cas, à lancer une par une sur ton Mac (serveur déjà démarré) :

**1. 400 — Host manquant**
Curl ajoute toujours `Host` automatiquement, donc il faut passer par `nc` en HTTP/1.0 minimal pour le forcer à manquer :
```bash
printf 'GET / HTTP/1.1\r\n\r\n' | nc -w 2 localhost 8080
```

**2. 400 — doublon de header Content-Length**
```bash
printf 'POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nContent-Length: 5\r\n\r\nhello' | nc -w 2 localhost 8080
```

**3. 400 — doublon de header Host**
```bash
printf 'GET / HTTP/1.1\r\nHost: localhost\r\nHost: localhost\r\n\r\n' | nc -w 2 localhost 8080
```

**4. 414 — URI trop longue (>8192 octets)**
```bash
curl -i "http://localhost:8080/$(python3 -c 'print("a"*9000)')"
```

**5. 431 — headers trop volumineux (>32768 octets au total)**
```bash
{
  printf 'GET / HTTP/1.1\r\n'
  printf 'Host: localhost\r\n'
  python3 -c 'print("X-Custom: " + "a"*33000 + "\r")'
  printf '\r\n'
} | nc -w 2 localhost 8080
```

**6. Path traversal — tentative d'évasion du dossier racine**
```bash
curl -i "http://localhost:8080/../../../etc/passwd"
curl -i "http://localhost:8080/..%2f..%2f..%2fetc%2fpasswd"
```

Pour ce dernier, attendu : soit un `404` (le chemin résolu ne pointe nulle part de valide), soit un `403`, mais **jamais** le contenu réel de `/etc/passwd`. Si jamais tu voyais apparaître du contenu système dans la réponse, ce serait une vraie faille à corriger avant l'oral — dis-le-moi immédiatement si c'est le cas.






## Rappel rapide des flags curl utilisés

| Flag | Effet |
|---|---|
| `-i` | affiche les headers de réponse + le body |
| `-X <METHODE>` | force la méthode HTTP (POST, DELETE, FOOBAR...) |
| `--data "..."` | envoie un corps de requête (POST) |
| `-c fichier.txt` | sauvegarde les cookies reçus dans ce fichier |
| `-b fichier.txt` | renvoie les cookies lus depuis ce fichier |
| `-L` | suit automatiquement les redirections (301/302) |
