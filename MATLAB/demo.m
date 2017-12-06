% Demo script showing extraction of ExHoG feature for a 128 x 64 (h x w)
% pixel window

% Thanks to Piotr Dollar for his Matlab Toolbox(v3.01) which provided
% functions for easy computation of HOG. Those functions have been modified
% to enable generation of ExHoG features. The function first calls
% gradientMagS to generate the gradient magnitude and direction matrices.
% Following which gradientHistEx is called upon to generate the ExHoG
% feature for the defined window The block overlap is default 50%. Cannot
% be changed as yet.

% Parameter initialization for ExHoG
cellsize = 8; % Block size is made up of 2 x 2 cells
nbin = 18; % Number of ExHoG bins
softBin = 1;
useHog = 1;
clip = 0.08; % As selected in TIP Paper

% Image from INRIA Training (160 x 96 pixel in size)
img = single(imread('crop001158a.png'))/255;

% Pre-processing
img = sqrt(img);
[d1,d2,~] = size(img);
[M,O] = gradientMagS(img); % Find signed gradients (magnitude and direction)

% Extract the inner 128 x 64 pixel window
M = M((d1-128)/2+1:end-(d1-128)/2,(d2-64)/2+1:end-(d2-64)/2);
O = O((d1-128)/2+1:end-(d1-128)/2,(d2-64)/2+1:end-(d2-64)/2);

% This returns a sparse 16 x 8 x 72 matrix(with some histograms zero-valued
% - these can be removed using the following steps to map to a 1 x d
% vector) Call to the mex function to generate the ExHoG feature
feature = gradientHistEx(M,O,cellsize,nbin,softBin,useHog,clip); 

% Extract relevant ExHoG feature (remove zero-valued histograms) Just an
% example. If the window size or cellsize parameter is changed, the valid
% cell histograms will change accordingly.
feature_interior = feature(2:15,2:7,:); %interior 14 x 6 region
[d1,d2,d3] = size(feature_interior);
feature_interior = reshape(feature_interior,d1*d2*d3,1);
feature_bound(1,:,:) = [feature(1,2:7,1:18) feature(1,2:7,37:54)]; %top row
[d1,d2,d3] = size(feature_bound);
feature_bound = reshape(feature_bound,d1*d2*d3,1);
feature_interior = [feature_interior; feature_bound];
clear feature_bound;
feature_bound(1,:,:) = [feature(16,2:7,19:36) feature(16,2:7,55:72)]; %bottom row
[d1,d2,d3] = size(feature_bound);
feature_bound = reshape(feature_bound,d1*d2*d3,1);
feature_interior = [feature_interior; feature_bound];
clear feature_bound;
feature_bound(:,1,:) = feature(2:15,1,1:36); %first column
[d1,d2,d3] = size(feature_bound);
feature_bound = reshape(feature_bound,d1*d2*d3,1);
feature_interior = [feature_interior; feature_bound];
clear feature_bound;
feature_bound(:,1,:) = feature(2:15,8,37:72); %last column
[d1,d2,d3] = size(feature_bound);
feature_bound = reshape(feature_bound,d1*d2*d3,1);
feature_interior = [feature_interior; feature_bound];
clear feature_bound;
feature_bound(1,1,:) = feature(1,1,1:18); %top left corner
[d1,d2,d3] = size(feature_bound);
feature_bound = reshape(feature_bound,d1*d2*d3,1);
feature_interior = [feature_interior; feature_bound];
clear feature_bound;
feature_bound(1,1,:) = feature(1,8,37:54); %top right corner
[d1,d2,d3] = size(feature_bound);
feature_bound = reshape(feature_bound,d1*d2*d3,1);
feature_interior = [feature_interior; feature_bound];
clear feature_bound;
feature_bound(1,1,:) = feature(16,1,19:36); %bottom left corner
[d1,d2,d3] = size(feature_bound);
feature_bound = reshape(feature_bound,d1*d2*d3,1);
feature_interior = [feature_interior; feature_bound];
clear feature_bound;
feature_bound(1,1,:) = feature(16,8,55:72); %bottom right corner
[d1,d2,d3] = size(feature_bound);
feature_bound = reshape(feature_bound,d1*d2*d3,1);
feature_interior = [feature_interior; feature_bound];

feature = feature_interior; % Final ExHoG feature