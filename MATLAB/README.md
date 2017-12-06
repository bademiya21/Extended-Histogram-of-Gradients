Author: Amit Satpathy (amit0001@gmail.com)

CAA 16122013

Version 1.0

Note that this code is only for academic use. Please contact the author, Nanyang
Technological University or Agency for Science, Technology and Research for commercial 
use.

Codes are based on the following papers:

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

If you are only using ExHoG in your work, kindly cite [1] - [4] in your publications. If 
you are using ExHoG + APCA in your work, kindly cite [1] and [3].

IMPORTANT NOTE

The code is provided as it is. Inside, Piotr Dollar's modified toolbox which contains 
ExHoG generation functions are provided. This version is modified from Version 3.01. As 
of writing this txt, the current version of Dollar's toolbox is 3.24. If time permits, I
will update the modified toolbox to the latest version of Piotr's Toolbox.

If you find any bugs, please inform me and I will fix them.

The codes work fine for Windows-based MATLAB. It has NOT been tested for Linux systems.

INSTALLATION

First, you will need to install the modified Piotr's MATLAB Toolbox. To do so, just do the 
following:

1)	Simply unzip, then add all directories to the Matlab path: 
	>> addpath(genpath('c:\piotrtoolboxmod_v3.01')); savepath;

2)	Run the compile script for the mex files: 
	>> toolboxCompile; 
  
Please install a C++ compiler (preferably Visual Studio) to compile the mex files for 
use.
 
USAGE
 
Please refer to the MATLAB script file, demo.m to view the procedures.
 
EXCEPTIONS
 
This code only contains the ExHoG extraction. The APCA procedure described in [1] and [3]can
be easily implemented in MATLAB using existing MATLAB functions and hence, support is not
provided.

Results produced in [1] - [4] were done using C/C++ versions of the code. Results may vary
slightly when using MATLAB versions.
