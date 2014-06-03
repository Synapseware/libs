libs
====

## Synapseware Libraries
Includes drivers for specific chips and more.



## Instructions
This repo is generally intended to be used as submodule in other repositories.  Here's how to set it up:
* Initialize the submodule in your repo:
```
* git submodule add git@github.com:Synapseware/libs.git libs/
```
* Make sure the new submodule has content:
```
  * git submodule update --init --recursive
```
  * You might need to make sure you're on 'master' or an appropriate branch.
* Be sure to commit the libs/ file and the .gitmodules file.  Git sees the submodules in the owning repo as a file with the same name as the directory.
