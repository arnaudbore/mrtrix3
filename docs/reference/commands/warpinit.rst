.. _warpinit:

warpinit
===================

Synopsis
--------

Create an initial warp image, representing an identity transformation

Usage
--------

::

    warpinit [ options ]  template warp

-  *template*: the input template image.
-  *warp*: the output warp image.

Description
-----------

This is useful to obtain the warp fields from other normalisation applications, by applying the transformation of interest to the warp field generated by this program.

The image generated is a 4D image with the same spatial characteristics as the input template image. It contains 3 volumes, with each voxel containing its own x,y,z coordinates.

Note that this command can be used to create 3 separate X,Y,Z images directly (which may be useful to create images suitable for use in the registration program) using the following syntax:

  $ warpinit template.mif warp-[].nii

Options
-------

Standard options
^^^^^^^^^^^^^^^^

-  **-info** display information messages.

-  **-quiet** do not display information messages or progress status.

-  **-debug** display debugging messages.

-  **-force** force overwrite of output files. Caution: Using the same file as input and output might cause unexpected behaviour.

-  **-nthreads number** use this number of threads in multi-threaded applications (set to 0 to disable multi-threading)

-  **-failonwarn** terminate program if a warning is produced

-  **-help** display this information page and exit.

-  **-version** display version information and exit.

--------------



**Author:** J-Donald Tournier (jdtournier@gmail.com)

**Copyright:** Copyright (c) 2008-2017 the MRtrix3 contributors.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, you can obtain one at http://mozilla.org/MPL/2.0/.

MRtrix is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

For more details, see http://www.mrtrix.org/.


