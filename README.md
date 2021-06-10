# v_nom

This repository is a snapshot of [Netlab's Voyager NOM](http://trac.netlabs.org/v_nom) source code and is **no longer maintained**.

You may also need:

* https://github.com/stain/v_nom/
* https://github.com/stain/v_desktop/
* https://github.com/stain/v_triton/
* https://github.com/stain/v_doc/

## License/copyright

License and copyright was not explicit across the original source repository, but where in-line, is indicated as Dual-license CDDL 1.0/LGPL 2.1, e.g.:


```
/* ***** BEGIN LICENSE BLOCK *****
* Version: CDDL 1.0/LGPL 2.1
*
* The contents of this file are subject to the COMMON DEVELOPMENT AND
* DISTRIBUTION LICENSE (CDDL) Version 1.0 (the "License"); you may not use
* this file except in compliance with the License. You may obtain a copy of
* the License at http://www.sun.com/cddl/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
* for the specific language governing rights and limitations under the
* License.
*
* The Original Code is "NOM" Netlabs Object Model
*
* The Initial Developer of the Original Code is
* netlabs.org: Chris Wohlgemuth <cinc-ml@netlabs.org>.
* Portions created by the Initial Developer are Copyright (C) 2005-2007
* the Initial Developer. All Rights Reserved.
*
* Contributor(s):
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU Lesser General Public License Version 2.1 (the "LGPL"), in which
* case the provisions of the LGPL are applicable instead of those above. If
* you wish to allow use of your version of this file only under the terms of
* the LGPL, and not to allow others to use your version of this file under
* the terms of the CDDL, indicate your decision by deleting the provisions
* above and replace them with the notice and other provisions required by the
* LGPL. If you do not delete the provisions above, a recipient may use your
* version of this file under the terms of any one of the CDDL or the LGPL.
*
* ***** END LICENSE BLOCK ***** */
```


Documentation adapted from <http://trac.netlabs.org/v_nom>:

# NOM the Netlabs Object Model 

NOM is the object model on which the [Voyager Desktop](http://voyager.netlabs.org/) is based.

## About 

The main feature of NOM is a release to release binary compatibility of classes.
This means that classes may be enhanced or added to the framework without
breaking other classes. These other classes may even be subclasses of the
modified class. Thanks to this feature it's possible to create binary only
extensions to the object model which won't break when the object model is
updated.

NOM allows to do object oriented programming with languages which are not designed as OO languages,
e.g. C.

## How to use NOM 

Classes are described in an Interface Definition Language (IDL). An IDL compiler interprets the file and
outputs binding and template files for the programming language in question. Right now only
C is supported but other bindings are possible e.g. C++ or Pascal.


## Building it

See [this page](http://trac.netlabs.org/v_nom/wiki/BuildNom) for building it on OS/2.

See [this page](http://trac.netlabs.org/v_nom/wiki/BuildNomDarwin) for building it on Darwin (OS X).

See [this page](http://trac.netlabs.org/v_nom/wiki/BuildNomWindows) for building it on Windows.

See [this page](http://trac.netlabs.org/v_nom/wiki/BuildNomLinux) for building it on Linux.

