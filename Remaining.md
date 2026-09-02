<!-- Auditer les custom error pages, particulièrement les erreurs produites directement par Server. -->
<!-- Tester positivement autoindex on. -->
Petit détail : le ".." apparaît aussi dans le listing. Ce n’est pas bloquant pour la validation de la feature, mais on pourra éventuellement décider plus tard de le filtrer pour un rendu plus propre.
<!-- Tester index configurable par location. -->
<!-- Tester une redirection configurée. -->
<!-- Tester upload_enabled + upload_path explicitement. -->
OK mais attention, si POST autorisé et upload_enabled == off, le Server upload actuellement le fichier dans le dossier par défaut (www/uploads). Vérifier que c'est ce qu'on veut, l'autre option serait d'interdire ce fallback
<!-- Tester 405 Method Not Allowed. -->
<!-- Auditer les configs invalides et vérifier que le serveur échoue proprement. -->
<!-- Faire quelques derniers tests HTTP malformés, sans chercher à implémenter toute la RFC. -->
<!-- Valgrind + FDs + processus CGI + stress final. -->

make fclean && make, zéro warning, puis tester depuis un clone/repository propre.

<!-- Préparer une config de démonstration couvrant toutes les fonctionnalités 
Le sujet exige explicitement de fournir les configs et fichiers nécessaires pour les démontrer pendant l'évaluation. -->

Préparer la soutenance orale.