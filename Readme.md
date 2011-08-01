Installation
=====================

Don't just clone the repo. This project needs to be located in
a firebreath/projects directory, as it uses the Firebreath framework.

To do this:

    # clone the firebreath repo, dev branch. its pretty stable.
    git clone git://github.com/firebreath/FireBreath.git firebreath-dev

    # make the subdirectory for sockit
    mkdir firebreath-dev/projects

    # clone the sockit repo
    git clone git@github.com:sockit/sockit.git firebreath-dev/projects/sockit

    # do some stuff here (you can build from here too).
    cd firebreath-dev/projects/sockit

Or, all at once now:

    git clone git://github.com/firebreath/FireBreath.git firebreath-dev && mkdir firebreath-dev/projects && 
    git clone git@github.com:sockit/sockit.git firebreath-dev/projects/sockit && cd firebreath-dev/projects/sockit


See sockit.github.com and firebreath.org for information about the plugin, and how to build it yourself.

For the impatient, run the appropriate build script for your platform. The resulting
binaries can be found in ../../build/projects/sockit/<buildtype>/

where <buildtype> is DEBUG by default.
