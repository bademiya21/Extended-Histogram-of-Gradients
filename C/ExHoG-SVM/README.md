Author: Amit Satpathy (amit00001@gmail.com)

Note that this code is only for academic use. Please contact the author, Nanyang
Technological University or Agency for Science, Technology and Research for commercial 
use.

Thanks to Navneet Dalal for providing the original source files for HOG. Those 
files have been modified for ExHoG implementation. His original source files can
be downloaded from http://pascal.inrialpes.fr/soft/olt/learcode.zip.

The following scientific publications contain the original descriptions of the 
method implemented in this Software:

[1] Amit Satpathy, Xudong Jiang and How-Lung Eng, “Human Detection by Quadratic 
Classification on Subspace of Extended Histogram of Gradients”, IEEE Transactions on Image
Processing, vol.23, no.1, pp.287-297, Jan, 2014.

[2] Amit Satpathy, Xudong Jiang and How-Lung Eng, “Visual Object Detection by Parts-based 
Modeling Using Extended Histogram of Gradients”, Proc. of the 2010 IEEE Int. Conf. on Image 
Processing. (ICIP’2013), September, 2013, Melbourne, Australia.

[3] Amit Satpathy, Xudong Jiang and How-Lung Eng, “Extended Histogram of Gradients with 
Asymmetric Principal Component and Discriminant Analyses for Human Detection”, Proc. of 
the IEEE Canadian Conf. on Computer and Robot Vision (CRV’2011), pp. 64-71, May, 2011, 
St. John’s, Canada.

[4] Amit Satpathy, Xudong Jiang and How-Lung Eng, “Extended Histogram of Gradients Feature 
for Human Detection”, Proc. of the 2010 IEEE Int. Conf. on Image Processing. (ICIP’2010),
pp. 3473 -3476, September, 2010, Hong Kong.

If you are only using ExHoG in your work, kindly cite [1] - [4] in your publications.

IMPORTANT NOTE
The code below is provided as it is. 

If you find some bugs, please inform me and I will fix them. 

WINDOWS SUPPORT
This code will NOT work in Windows. It is meant for Linux-based systems.
 

SVM LEARN FILES
This code does not build svm_learn binary. For that download

http://pascal.inrialpes.fr/soft/olt/svmdense.tar.gz
and build a svm_learn binary.

GENERAL INFORMATION
This code implements a multiscale ExHoG detector for visual object
recognition and notably for human detection using linear SVM. It requires three 
external libraries:

-- Imlib2       for reading/writing images
-- Blitz        for efficient array processing
-- Boost        for many other general c++ utilities not present in ISO C++

Some portions of the code (notably the color conversion routines)
contain sources adapted from the Vigra Image Processing library
(http://kogs-www.informatik.uni-hamburg.de/~koethe/vigra/). Vigra is
templated image processing library distributed under the MIT X11
license. Where ever Vigra code has been used, this is mentioned in the
corresponding header file. A license file is included in the
distribution.

INSTALLATION
First install the development versions (*-devel RPM's or similar) of
the three external libraries as follows, making sure that the headers
and libraries are in your standard search paths :

Imlib2  - version 1.2.2 tested
Boost   - version 1.33.1 or later
Blitz   - version 0.9 tested (installed from source as no RPM's are
          currently available)

Then unpack the learcode.tar.gz file, change to it top level directory
and run ./configure. If the libraries were installed in nonstandard
places you can tell configure where to find them using, e.g.

  ./configure --with-blitz=/usr/local --with-boost=/usr/local ...
  
To compile the code run "sudo make" in the shell. Transfer the following binaries
into the bin folder of OLTbinaries (downloadable from 
http://pascal.inrialpes.fr/soft/olt/OLTbinaries.zip) for training and testing 
- classify_rhog, dump_rhog, dump4svmlearn and svm_learn (from svmdense). 

A demo script file, runall_INRIA_ExHoG_fppi.sh, using INRIA data set as an example has 
been provided. This code is modified to provide results for per-image methodology.
