#<cldoc:index>

Semigroups++

# Semigroups++ Version 0.0.1

This is the source code documentation for Semigroups++ produced using
[cldoc](https://github.com/jessevdk/cldoc). This documentation can be
compiled by running `make doc` in the `semigroupsplusplus`
directory, and the tests can be run by doing `make test` in the
`semigroupsplusplus` directory.

The algorithms and data structures in this version of Semigroups++ are
based on [Algorithms for computing finite semigroups](https://www.irif.fr/~jep/PDF/Rio.pdf), 
[Expository Slides](https://www.irif.fr/~jep/PDF/Exposes/StAndrews.pdf), and 
[Semigroupe 2.01](https://www.irif.fr/~jep/Logiciels/Semigroupe2.0/semigroupe2.html) 
by Jean-Eric Pin.

The Semigroups++ library is used in the [Semigroups package for GAP](http://gap-packages.github.io/Semigroups/).
The development version is available on 
[Github](https://github.com/james-d-mitchell/semigroupsplusplus).

Some of the features of 
[Semigroupe 2.01](https://www.irif.fr/~jep/Logiciels/Semigroupe2.0/semigroupe2.html) 
are not yet implemented in the Semigroups++ library, this is a work in
progress. Missing features include those for:

* Green's relations, or classes
* finding a zero
* minimal ideal, principal left/right ideals, or indeed any ideals
* inverses
* local submonoids
* the kernel
* variety tests.

These will be included in a future version. 

At present the Semigroups++ library
seems to perform slightly worse than 
[Semigroupe 2.01](https://www.irif.fr/~jep/Logiciels/Semigroupe2.0/semigroupe2.html)
due in part to the fact that it is somewhat more versatile.

The Semigroups++ library also has some advantages over 
[Semigroupe 2.01](https://www.irif.fr/~jep/Logiciels/Semigroupe2.0/semigroupe2.html):

* there is a (hopefully) convenient C++ API, which makes it relatively easy to
  create, and manipulate semigroups and monoids
* there are some multithreaded methods for semigroups and their congruences
* you do not have to know/guess the size of a semigroup or monoid before you
  begin
* Semigroups++ supports more types of elements than 
[Semigroupe 2.01](https://www.irif.fr/~jep/Logiciels/Semigroupe2.0/semigroupe2.html):
* it is relatively straightforward to add support for further types of elements
  and semigroups
* it is possible to enumerate a certain number of elements of a semigroup or
  monoid (say if you are looking for an element with a particular property), to
  stop, and then later start the enumeration again at a later point.
* you can instantiate as many semigroups and monoids as you can fit in memory
* it is possible to add more generators after a semigroup or monoid has been
  constructed, without lossing or having to recompute any information that was
  previously known
* Semigroups++ contains a rudimentary implementation of the Todd-Coxeter
  algorithm for finitely presented semigroups, which can also be used to
  compute congruence of a (not necessarily finitely presented) semigroup or
  monoid.

If you find any problems with Semigroups++ or have any suggestions for features
that you'd like to see please use the 
[issue tracker](https://github.com/james-d-mitchell/semigroupsplusplus/issues).
