Installation
=====================

Don't just clone the repo. This project needs to be located in
a firebreath/projects directory, as it uses the Firebreath framework.

To do this:

    # Clone the firebreath repo, dev branch. Its pretty stable. If this branch proves to be unstable,
    # try one of the firebreath-1.x branches. This plugin was originally developed on the firebreath-1.5
    # branch, so that one should work.
    git clone git://github.com/firebreath/FireBreath.git firebreath-dev

    # Make the subdirectory for sockit
    mkdir firebreath-dev/projects

    # Clone the sockit repo
    git clone git@github.com:sockit/sockit.git firebreath-dev/projects/sockit

    # Do some development here (you can build and package the project from here).
    cd firebreath-dev/projects/sockit

Or, all at once now:

    git clone git://github.com/firebreath/FireBreath.git firebreath-dev && mkdir firebreath-dev/projects && 
    git clone git@github.com:sockit/sockit.git firebreath-dev/projects/sockit && cd firebreath-dev/projects/sockit


Building
====================

See sockit.github.com and firebreath.org for information about the plugin, and how to build it yourself. Extensive documentation on 
building and packaging the project can be found at <a href="http://sockit.github.com/developers.html">http://sockit.github.com/developers.html.

Run the appropriate build script for your platform. The resulting binaries can be found in <code>bin\\<platform></code> 
