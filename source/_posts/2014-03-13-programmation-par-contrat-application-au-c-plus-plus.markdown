---
layout: post
title: "Programmation par Contrat -- Application au C++"
date: 2014-03-13 17:47:34 +0100
comments: true
categories: C++
published: false
footer: true
---

Intro...

# Les Erreurs

En développement, il y a toujours des problèmes qui vont venir nous
enquiquiner. Certains correspondront à des problèmes plausibles induits par le
contexte (fichiers invalides, connexions réseau coupées, utilisateurs qui
saisissent n'importe quoi, ...), d'autres seront des _erreurs de programmation_.

Dans la suite de ce billet, je vais traiter des erreurs de programmation
uniquement.

## Leurs types

Quand on parle d'erreur de programmation, les premières qui vont nous venir à
l'esprit sont les erreurs de syntaxe (point-virgule oublié), ou de grammaire
(types non respectés). Ces erreurs-ci, les langages compilés vont nous les
signaler. On peut considérer qu'il est impossible de livrer un exécutable sur
une plateforme qui n'a pas passé cette phase de vérification.

Il existe de nombreuses autres erreurs de programmation qu'aucun compilateur ne
signalera jamais. On peut se tromper dans la conception ou la retranscription
d'un algorithme, et ainsi renvoyer des résultats numériques abérants. On peut
aussi faire des suppositions totalement erronées, comme traiter les lignes d'un
fichier qui n'existe pas, ou exploiter un élément après sa recherche
infructueuse dans une liste, ... Les plus classiques sont les accès
hors-bornes, et tous les autres problèmes de déréférencement de pointeur nul
et de _dangling pointer_.

Bien sûr, un fichier qui n'existe pas est une erreur de contexte. Mais
réaliser un traitement sur un fichier sans vérifier préalablement qu'il existe
est une erreur de programmation.

## Ce que l'on en fait

Les erreurs qui bloquent la compilation, on n'a pas trop d'autre choix que de
les corriger. Les autres erreurs ... souvent, pas grand chose n'en est fait.
Elles sont là, elles trainent jusqu'à ce qu'elles soient trouvées, puis
corrigées. Les pires d'entre-elles ne sont jamais détectées. C'est souvent le
cas des erreurs numériques, ou des fichiers que l'on croit avoir ouverts.

Dans les meilleurs de mes mondes, on fait en sorte de ne pas pouvoir compiler
quand on est face à une erreur de programmation. Les assertions statiques nous
aideront pour cela.
Ou on peut appliquer des petites recettes dont le principe chapeau consiste à
confier nos invariants au compilateur. Par exemple, on évite de disposer de variables dans
des états non pertinents (FAQ C++ dvpz), on utilise des références à la place de pointeurs
quand on sait que l'on est censés disposer de _liens_ non nuls, on annote comme
**transférables** les types dont les reponsables changent (cf un prochain
billet), ...

Pour les autres cas, Bertrand Meyer a jeté les bases d'un outil, la
_programmation par contrat_, et le C nous offre un second outil, les _assertions_.


# La programmation par contrat

Les contrats, dans la programmation, servent à poser les bases de qui est censé
faire quoi. Par exemple, la fonction `sqrt(x)` ne prend que des paramètres
numériques positifs _x_, et elle renvoie des nombres toujours positifs qui
vérifient _result = x²_. On retrouve la notion de _domaine de définition_ des
_fonctions_ en mathématiques.

Dit autrement, si on respecte le contrat d'appel d'une fonction (on parle de
ses _pré-conditions_), cette fonction est censée nous garantir respecter son
contrat de sortie (on parle de _post-conditions_). Si les pré-conditions ne
sont pas respectées, les post-conditions (à commencer par le bon déroulement de
la fonction) pourront ne pas être respectées : la fonction est libre de faire
comme elle l'entend.

On peut se demander à quoi ça sert. En effet, si on passe un nombre négatif à
`sqrt` et qu'elle plante, on n'est pas plus avancés. Le bug est toujours là. Et
pourtant, nous avons fait un pas énorme : nous avons formalisé les contrats de
`sqrt`. Nous disposons de spécifications précises, et d'une documentation qui
pourra accompagner le code -- [Doxygen](http://doxygen.org), mets à notre
disposition les tags `@pre`, `@post`, et `@invariant` pour documenter nos
contrats. 

Comment aller plus loin et trouver les erreurs de programmation : assert, et
analyse statique

## Les trois contrats

## Acteurs et responsabilités

## contrats commerciaux...

# Application au C++

assert

IP C++

propal C++14/17

snippet pour invariant

NVI

doxygen
 
 analyse statique

# PpC & POO -> LSP

# Références
- Meyer
- Dunski
- IP C++
- n \d
- billets sur altblogdev


