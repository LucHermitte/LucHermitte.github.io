---
layout: post
title: "Programmation par Contrat 1/3 -- un peu de théorie"
date: 2014-03-13 17:47:34 +0100
comments: true
categories: C++
published: true
footer: true
---

Cela faisait un moment que je voulais partager mes conclusions sur la
_Programmation par Contrat_, et en particulier comment l'appliquer au C++.

Voici un premier billet qui aborde l'aspect théorique. Dans un second billet,
je traiterai des _assertions_. Et en guise de conslusion, je présenterai des
techniques d'application de la PpC au C++ que j'ai croisées au fil des ans.

## I- Les Erreurs

En développement, il y a toujours des problèmes qui vont venir nous ennuyer.
Certains correspondront à des problèmes plausibles induits par le contexte
(fichiers invalides, connexions réseau coupées, utilisateurs qui saisissent
n'importe quoi, ...), d'autres seront des _erreurs de programmation_.

Dans la suite de ce billet, je vais principalement traiter du cas des erreurs
de programmation.

### Les types d'erreurs de programmation

Quand on parle d'erreur de programmation, les premières qui vont nous venir à
l'esprit sont les erreurs de syntaxe (point-virgule oublié), ou de grammaire
(types non respectés). Ces erreurs-ci, les langages compilés vont nous les
signaler. On peut considérer qu'il est impossible de livrer un exécutable sur
une plateforme qui n'a pas passé cette phase de vérification.

Il existe de nombreuses autres erreurs de programmation qu'aucun compilateur ne
signalera jamais. On peut se tromper dans la conception ou la retranscription
d'un algorithme, et ainsi renvoyer des résultats numériques aberrants. On peut
aussi faire des suppositions totalement erronées, comme traiter les lignes d'un
fichier qui n'existe pas, ou exploiter un élément après sa recherche
infructueuse dans une liste, ... Les plus classiques sont les accès
hors bornes, et tous les autres problèmes de déréférencement de pointeur nul
et de [_dangling pointer_](http://en.wikipedia.org/wiki/Dangling_pointer).

Bien sûr, un fichier qui n'existe pas est une erreur de contexte. Mais
réaliser un traitement sur un fichier sans vérifier préalablement qu'il existe
est une erreur de programmation. La différence est subtile. J'y reviendrai plus
[loin](#ProgrammationDefensive).

### Ce que l'on en fait

Les erreurs qui bloquent la compilation, on n'a pas trop d'autre choix que de
les corriger. Les autres erreurs ... souvent, pas grand chose n'en est fait.
Elles sont là, elles trainent jusqu'à ce qu'elles soient trouvées, puis
corrigées. Les pires d'entre-elles ne sont jamais détectées. C'est souvent le
cas des erreurs numériques, ou des fichiers que l'on croit avoir ouverts.

Dans les meilleurs de mes mondes, on fait en sorte de ne pas pouvoir compiler
quand on est face à une erreur de programmation. Les assertions statiques nous
aideront en cela.  
On peut aussi appliquer des petites recettes dont le principe chapeau consiste
à confier nos invariants au compilateur. Par exemple, on évite de disposer de
variables dans des états non pertinents (FAQ C++ dvpz), on utilise des
références à la place de pointeurs quand on sait que l'on est censés disposer
de _liens_ non nuls, on annote comme **transférables** les types dont les
responsables changent (_cf._ un prochain billet), on fait en sorte de ne pas
pouvoir additionner des distances avec des masses (_cf._
[boost.unit](http://boost.org/libs/units)), ...

Pour les autres cas, Bertrand Meyer a jeté les bases d'un outil, la
_programmation par contrat_, et le C nous offre un second outil, les _assertions_.


## II- La programmation par contrat

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
`sqrt`. Nous disposons de spécifications précises, et d'une
[documentation](#Documentation) qui pourra accompagner le code.

Heureusement, nous pouvons aller bien plus loin. Nous pouvons profiter des
contrats exprimés pour les confier à des outils d'analyse statique qui se
chargeront de vérifier les ruptures possibles en explorant un maximum de
chemins d'exécution possibles. Nous pouvons aussi marquer le code avec des
assertions représentatives des contrats identifiés pour repérer les ruptures de
contrats en phase de tests et développement.

### Les trois contrats

La PpC définit trois contrats :
#### Les pré-conditions 
Elles sont le pendant des _domaines de définition_ des fonctions mathématiques.
Si l'état du système vérifie les pré-conditions d'une fonction à l'instant de
son appel, alors la fonction est censée se dérouler correctement et de façon
_prévisible_ (je simplifie).

Typiquement, l'_état du système_ correspondra aux paramètres de la fonction,
qu'ils soient explicites, ou implicites (`this`), mais aussi à toutes les
globales accessibles.

#### Les post-conditions 
Les post-conditions sont les garanties qu'on a sur le résultat d'une fonction
si les pré-conditions sont remplies et que la fonction s'est exécutée
correctement.

#### Les invariants
Il y a plusieurs natures d'invariants. On va parler d'invariants pour des zones
de codes durant lesquelles une propriété sera vraie :

* un _invariant de boucle_ correspondra à ce qui est toujours vrai à
  l'intérieur de la boucle (p.ex. que `i < N` dans le cas d'une boucle `for`) ;
* une variable devrait toujours avoir pour invariant : _est utilisable, et est
  dans un état cohérent et pertinent_ (_cf._ FAQ C++ dvpz) ;
* un _invariant de classe_ est une propriété toujours observable depuis
  l'extérieur des instances de la classe -- p.ex. une séquence triée garantira
  que tous les éléments de la séquence sont toujours ordonnés lorsque le code
  utilisant la séquence cherche à y accéder, cependant ponctuellement, le temps
  de l'insertion d'un nouvel élément l'invariant de la classe n'a pas à être
  vérifié depuis les fonctions internes de la séquence ;
* une _référence_ est généralement acceptée en C++ comme un pointeur avec pour
  invariant une garantie de non-nullité.


### Acteurs et responsabilités

On peut dans l'absolu distinguer autant d'acteurs dans l'écriture d'un code
que de fonctions qui interviennent. Prenons le bout de code suivant :

```c++
double metier() {                  // écrit par l'Intégrateur
   const double i = interrogeES(); // écrit par le responsable UI
   return sqrt(i);                 // écrit par le Mathématicien
}
```

Nous pouvons distinguer trois acteurs : 

* le responsable UI, qui écrit `interrogeES`
* le Mathématicien, qui écrit `sqrt`
* et l'Intégrateur, qui intègre tout cela ensemble lorsqu'il écrit `metier`.

`sqrt` a un contrat simple : le nombre reçu doit être positif. Si l'appel à
`sqrt` échoue (plantage, résultat renvoyé aberrant, ...) tandis que le nombre
passé en paramètre est bien positif, alors le Mathématicien est responsable du
problème. En effet, bien que les pré-conditions de `sqrt` sont bien vérifiées,
ses post-conditions ne le sont pas : `sqrt` ne remplit pas sa part du contrat.

Si `i` n'est pas positif, alors le Mathématicien ne peut pas être tenu pour
responsable de quoi que ce soit. La faute incombe au code client de `sqrt`.

Maintenant, tout va dépendre si `interrogeES` dispose d'une post-condition sur
ses sorties du type _renvoie un nombre positif_. Dans ce cas, la rupture de
contrat est à ce niveau, et le responsable UI est responsable de l'erreur de
programmation. L'Intégrateur est dans son droit d'enchainer
`sqrt(interrogeES())`. C'est exactement la même chose que
`sqrt(abs(whatever))`, personne n'irait accuser l'Intégrateur de ne pas faire
son boulot.

En revanche, si `interrogeES` n'a aucune post-condition telle que le nombre
renvoyé sera positif, alors l'Intégrateur est responsable au moment de l'intégration de
s'assurer que ce qu'il va passer à `sqrt` soit bien positif. Une correction
typique serait :

```c++
double metier() {                  // écrit par l'Intégrateur
   const double i = interrogeES(); // écrit par le responsable UI
   if (i <0)
       throw std::runtime_error("invalid input obtained ...");
   return sqrt(i);                 // écrit par le Mathématicien
}
```

Remarquez, que l'Intégrateur est alors face à une erreur de contexte
(/_runtime_) et nullement face à une erreur de programmation. Il est alors en
droit de lever une exception, ou de boucler jusqu'à obtenir quelque chose qui
lui permette de continuer. Sans cela nous aurions été face à une erreur de
programmation commise par l'Intégrateur.


En résumé :

* la responsabilité de vérifier les pré-conditions d'une fonction échoie au
  code client, ou au code qui alimente les entrées de la fonction appelée.
* la responsabilité de vérifier les post-conditions d'une fonction échoie à la
  fonction appelée.


NB: Jusqu'à présent je considérai seulement deux acteurs relativement aux
responsabilités. C'est Philippe Dunski qui m'a fait entrevoir le troisième
intervenant lors de ma relecture de son livre [[Dunksi2014](#Dunksi2014)].

### Contrats commerciaux... et licences

La programmation par contrat n'a pas vocation à avoir des répercutions légales
selon qui ne remplit pas son contrat. Cependant, il y a clairement une
intersection entre la PpC et les responsabilités légales.

Dans le cas où le responsable UI et le Mathématicien sont deux contractants de
l'Intégrateur. Ce que j'ai détaillé au paragraphe précédant est normalement
directement applicable. L'Intégrateur sera responsable vis-à-vis de son client
du bon fonctionnement de l'ensemble, mais le responsable UI et le Mathématicien
ont des responsabilités vis-à-vis de l'Intégrateur.

Si maintenant, le responsable UI ou le Mathématicien ne livrent plus des
[COTS](http://en.wikipedia.org/wiki/Commercial_off-the-shelf), mais des
bibliothèques tierces OpenSources ou Libres. À moins que l'Intégrateur ait pris
un contrat de maintenance auprès du responsable UI et du Mathématicien, il est
peu probable que le responsable UI ou le Mathématicien aient la moindre
responsabilité légale vis à vis de l'Intégrateur. 

L'Intégrateur est seul responsable vis-à-vis de son client. À lui de trouver des
contournements, ou mieux de corriger ces composants tiers et de reverser les
patchs à la communauté.

Mais je m'égare, ceci est une autre histoire.  Revenons à nos moutons.

## <a id="ProgrammationDefensive"></a>III- Programmation par Contrat ou Programmation Défensive ?

La _Programmation Défensive_ a pour objectif qu'un programme ne doit jamais
s'arrêter afin de pouvoir toujours continuer.

Bien que la PpC puisse être détournée pour faire de la programmation
défensive, ce n'est pas son objectif premier. La PpC ne fait que stipuler que
si un contrat est respecté, alors tout se passera bien. Si le contrat n'est pas
respecté tout peut arriver : on peut assister à des plantages plus ou moins
prévisibles, on peut produire des résultats erronés, on peut stopper
volontairement au point de détection des erreurs, et on peut aussi remonter
des exceptions.

Le choix de remonter des exceptions, depuis le lieu de la détection de la
rupture de contrat, est un choix de programmation défensive. C'est un choix que
j'assimile à une déresponsabilisation des véritables responsables.

Supposons une application qui lit un fichier de distances, et qui pour le besoin
de son métier calcule des racines carrées sur ces distances. L'approche de la 
_programmation défensive_ consisterait à vérifier dans la fonction `my::sqrt` que
le paramètre reçu est positif, et à lever une exception dans le cas contraire.

Ce qui donnerait :
```c++
double my::sqrt(double n) {
    if (n<0) throw std::domain_error("Negative number sent to sqrt");
    return std::sqrt(n);
}

void my::process(boost::filesystem::path const& file) {
    boost::ifstream f(file);
    if (!f) throw std::runtime_error("Cannot open "+file.string());
    double d;
    while (f >> d) {
        my::memorize(my::sqrt(d));
    }
}
```
Si un nombre négatif devait être présent dans le fichier, nous aurions droit à
l'exception *"Negative number sent to sqrt"*. Limpide, n'est-ce pas ? On ne sait
pas quel est le nombre, ni d'où il vient. Après une longue investigation pour
traquer l'origine ce nombre négatif, on comprend enfin qu'il faut instrumenter
`process` pour intercepter l'exception. Soit on fait le `catch` au niveau de la
fonction, et on sait dans quel fichier a lieu l'erreur, soit on encadre l'appel
à `my::sqrt` pour remonter plus d'informations. Et là ... on fait ce que le code
client aurait du faire dès le début : assurer que le contrat des fonctions
appelées est bien respecté.

Si on avait embrassé la PpC dès le départ, ce bout de code aurait ressemblé à :
```c++
double my::sqrt(double n) {
    assert(n>=0 && "sqrt can't process negative numbers");
    return std::sqrt(n);
}

void my::process(boost::filesystem::path const& file) {
    boost::ifstream f(file);
    if (!f) throw std::runtime_error("Cannot open "+file.string());
    double d;
    for (std::size_t l = 1 ; f >> d ; ++l) {
        if (d <= 0) 
            throw std::runtime_error(
                "Invalid negative distance " + std::to_string(d)
                +" at the "+std::to_string(l)
                +"th line in distances file "+file.string());
        my::memorize(my::sqrt(d));
    }
}
```
Cette fois-ci, nous aurions droit à un message non seulement plus explicite,
mais surtout bien plus utile : *"Invalid negative distance -28.15 at the 42th line of
distances file distances.txt"*.

Mes conclusions personnelles sur le sujet :

* Je préfère 100 fois la PpC à la Programmation Défensive : il faut assumer nos
  responsabilités et ne pas décharger nos utilisateurs de leurs devoirs.
* Il est possible de détourner la PpC pour faire de la Programmation Défensive
  ; p.ex. l'assertion pourrait être détournée en *Release* pour lever une
  exception. Plus sur le sujet dans le prochain billet.


### Comment reconnaitre des contrats ?

Il est important de le rappeler, les contrats tels que présentés ici sont
orientés vers la recherche des erreurs de programmation. C'est à dire, un code
qui ne respecte pas le contrats de ses divers constituants présente une erreur
de programmation.

En aucun cas une violation de contrat correspondra à une situation
exceptionnelle (et plausible), _cf._ [[Wilson2006]](#Wilson2006)

Il est également à noter qu'une vérification de contrat devrait pouvoir être
retirée d'un code source sans que son comportement ne soit impacté. En effet,
un programme dépourvu d'erreur de logique n'aura aucun contrat qui se fasse
violer, et la vérification devient superflue.

## IV- Parenthèse OO : PpC & _Principe de Substitution de Liskov_ (LSP)

Je ne rentrerai pas dans les détails du LSP. Je vous renvoie plutôt à la
[FAQ C++ de développez](http://cpp.developpez.com/faq/cpp/?page=L-heritage#Qu-est-ce-que-le-LSP),
ou à [[Dunksi2014](#Dunksi2014)]. Il faut retenir que le LSP
est un outil qui permet d'éviter de définir des hiérarchies de classes qui se
retourneront contre nous.

Le LSP est formulé relativement aux contrats des classes pour établir quand une
classe peut dériver (publiquement en C++) en toute quiétude d'une autre.
Le principe est que :

* les pré-conditions ne peuvent être qu'affaiblies, ou laissées telles quelles,
* les post-conditions ne peuvent être que renforcées, ou laissées telles
  quelles,
* et la classe fille ne peut qu'ajouter des invariants.

Dit comme cela, cela peut paraitre compliqué, et pourtant c'est très logique.  
Par exemple, une compagnie aérienne a des pré-requis sur les bagages acceptés
sans surcouts. Pour toutes, un bagage de 50x40x20cm sera toujours accepté. En
particulier, chez les compagnies low-costs. En revanche, les grandes compagnies
historiques (et non low-costs) affaiblissent cette pré-condition : on peut se
s'enregistrer avec un bagage bien plus gros sans avoir à payer de supplément
(certes il partira en soute).  
Même chose pour les post-conditions : nous n'avons aucune garantie d'estomac
rempli sans surcouts une fois à bord de l'avion. Sauf chez les compagnies
traditionnelles qui assurent en sortie un estomac non vide.  
Côté invariants, un rectangle immuable a tous ses côtés perpendiculaires, un
carré immuable a en plus tous ses côtés de longueur égale.

Parmi les conséquences du LSP, on pourra déduire qu'une _liste triée_ n'est pas
substituable à une _liste_, ou qu'un _carré non immuable_ n'est pas un
_rectangle non immuable_. Je vous renvoie à la littérature et/ou la FAQ pour
plus d'informations sur le sujet.


## V- Références

(à reformuler/dispatcher)

- <a id="Meyer1988"></a>[Meyer2000] -- [_Conception et programmation orientées objet_](http://www.editions-eyrolles.com/Livre/9782212122701/conception-et-programmation-orientees-objet) de Bertrand Meyer, Eyrolles, 1988, Seconde Édition parue en 2000
- <a id="Dunksi2014"></a>[Dunksi2014] -- [_Coder Efficacement -- Bonnes pratiques et erreurs à éviter (en C++)_](http://www.d-booker.fr/programmation-et-langage/157-coder-efficacement.html) de Philippe Dunski, D-Booker, Février 2014
- <a id="IPCpp"></a>[Wilson2004] -- _Imperfect C++_ de Matthew Wilson, Addisson-Wesley Professionnal, Octobre 2004.
- <a id="Wilson2006"></a>[Wilson2006] -- [_Contract Programming 101_](http://www.artima.com/cppsource/deepspace3.html), Matthew Wilson, artima, Janvier 2006.
- [n3753](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3753.pdf)
- Billets d'Andrzej  
  Il s'agit là d'une excellente série d'articles/billets sur la PpC.
  - http://akrzemi1.wordpress.com/2013/01/04/preconditions-part-i/
  - http://akrzemi1.wordpress.com/2013/02/11/preconditions-part-ii/
  - http://akrzemi1.wordpress.com/2013/03/13/preconditions-part-iii/
  - http://akrzemi1.wordpress.com/2013/04/18/preconditions-part-iv/
- Autres articles
  - [_When and How to Use Exception_](http://www.drdobbs.com/when-and-how-to-use-exceptions/184401836), Herb Sutter, August 01, 2004
  - http://blog.regehr.org/archives/1091
  - http://pempek.net/articles/2013/11/16/assertions-or-exceptions/
  - http://pempek.net/articles/2013/11/17/cross-platform-cpp-assertion-library/
  - [_Programmation par contrat, application en C++_](http://julien-blanc.developpez.com/articles/cpp/Programmation_par_contrat_cplusplus/), de Julien Blanc, 14 décembre 2009  
    Vous trouverez d'autres explications et d'autres techniques dans son
    article.


