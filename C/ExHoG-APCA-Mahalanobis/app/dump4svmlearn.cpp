/*
 * =====================================================================================
 *
 *       Filename:  dump4svmlearn.cpp
 *
 *    Description:  Convert dump_rhog output to binary svm_learn format.
 *
 *        Version:  1.0
 *        Created:  Tuesday 12 June 2007 01:37:07  IST IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Navneet Dalal
 *        Company:  Lear / INRIA
 *
 * =====================================================================================
 */

#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <math.h>

#include <lear/io/ioext.h>
#include <lear/blitz/blitzio.h>
#include <lear/io/fileheader.h>
#include <lear/io/biistream.h>
#include <lear/io/biostream.h>
#include <lear/util/customoption.h>
#include <lear/util/util.h>

#include <lear/cmdline.h>

#include "rawdescio.h"
#include "formatstream.h"
#include <cv.h>
#include <cxcore.h>


using std::cout; using std::cerr; using std::endl;
// ==========================================================================
// -------- Command typedefs        -----------------------------------------
// ==========================================================================
typedef std::vector<std::string>    FileListVector;

typedef int                         TargetType;

typedef blitz::Array<int,1>         Array1DInt;


enum FileFormat { BiSVMLight, SVMLight, BiMatlab, Matlab};

// ==========================================================================
// -------- Command line parameters -----------------------------------------
// ==========================================================================
static FileListVector posfile, negfile;
static std::string outfile;
static std::string outfile1;
static std::string outfile2;

static int maxposvec, maxnegvec, rednumofdim;
static int verbose;
static bool do_apca=false;
static bool bootstrap=false;
static FileFormat formatType;

/// length of feature vector
static int featureLength;

static int sizesamples,possize,negsize,steppos=0,stepneg=0;
static float wp, beta1, alpha1;

/// local feature vector cache
static Array1DType feature, feature_transf;

/// OpenCV data types
CvMat **possample, **negsample;

static void writeData(
        FormatOutStream<RealType,TargetType>* to, 
        const std::string& filename,  
        int& maxvec, 
        const TargetType target)
{// {{{
    RawDescIn desc(filename, verbose);

    if (featureLength <= 0) 
    {
        featureLength = desc.featureLength();
        // allocate space for computing norm and std
        feature.resize(featureLength);

    } else if (desc.featureLength() != featureLength) 
    {
        std::ostringstream mesg;
        mesg << "Unequal feature length in file " << filename
            <<". Expected feature length " << featureLength
            <<", found feature length " << desc.featureLength();
        throw lear::Exception("writeData()",mesg.str());
    }
    int size = desc.featureCount();
    if (maxvec > 0){
        size = std::min(maxvec,desc.featureCount());
        maxvec -=size;
    }
	
	for (int m= 0; m< size && desc; ++m) {
		desc.next(feature); 
		to->write(feature, target);
		if (verbose > 5){
			cout << "Read descriptor " << std::setw(5) << m << endl;
		}
	}
	//}
    if (verbose > 2) {
        cout << "Processed descriptors " << std::setw(7) << size ;
        if (verbose > 3) 
            cout << " from file " << filename;
        cout << endl;
    }
}// }}}

static void createPosDataMat(
        const std::string& filename,  
        int& maxvec)
{// {{{
    RawDescIn desc(filename, verbose);

    if (featureLength <= 0) 
    {
        featureLength = desc.featureLength();
        // allocate space for computing norm and std
        feature.resize(featureLength);

    } else if (desc.featureLength() != featureLength) 
    {
        std::ostringstream mesg;
        mesg << "Unequal feature length in file " << filename
            <<". Expected feature length " << featureLength
            <<", found feature length " << desc.featureLength();
        throw lear::Exception("writeData()",mesg.str());
    }
    int size = desc.featureCount();
    if (maxvec > 0){
        size = std::min(maxvec,desc.featureCount());
        maxvec -=size;
    }

    for (int m= 0; m< size && desc; ++m) {
    	desc.next(feature);
	// Create OpenCV array of postive samples
	for (int l=featureLength;l--;){
		cvSetReal2D(possample[0], l, m+steppos, feature(l));
	}
	if (verbose > 5){
            cout << "Read descriptor " << std::setw(5) << m << endl;
        }
    }

    steppos+=size;

    if (verbose > 2) {
        cout << "Processed descriptors " << std::setw(7) << size ;
        if (verbose > 3) 
            cout << " from file " << filename;
        cout << endl;
    }
}// }}}

static void createNegDataMat(
        const std::string& filename,  
        int& maxvec)
{// {{{
    RawDescIn desc(filename, verbose);

    if (featureLength <= 0) 
    {
        featureLength = desc.featureLength();
        // allocate space for computing norm and std
        feature.resize(featureLength);

    } else if (desc.featureLength() != featureLength) 
    {
        std::ostringstream mesg;
        mesg << "Unequal feature length in file " << filename
            <<". Expected feature length " << featureLength
            <<", found feature length " << desc.featureLength();
        throw lear::Exception("writeData()",mesg.str());
    }
    int size = desc.featureCount();
    if (maxvec > 0){
        size = std::min(maxvec,desc.featureCount());
        maxvec -=size;
    }

    for (int m= 0; m< size && desc; ++m) {
        desc.next(feature);
	// Create complete array of negative samples 
	for (int l=featureLength;l--;){ 
		cvSetReal2D(negsample[0], l, m+stepneg, feature(l));
	}
		      
        if (verbose > 5){
            cout << "Read descriptor " << std::setw(5) << m << endl;
        }
    }
	
    stepneg+=size;

    if (verbose > 2) {
        cout << "Processed descriptors " << std::setw(7) << size ;
        if (verbose > 3) 
            cout << " from file " << filename;
        cout << endl;
    }
}// }}}

// {{{ compute
static void compute() {
    FormatOutStream<RealType,TargetType>* to = 0;
    switch (formatType) {
        case BiSVMLight:
            to= new BiSVMLightOutStream<RealType, TargetType> (outfile);
            break;
        case SVMLight:
            to= new SVMLightOutStream<RealType, TargetType> (outfile);
            break;
        case Matlab:
            to= new MatlabOutStream<RealType, TargetType>    (outfile);
            break;
        case BiMatlab:
            to= new BiMatlabOutStream<RealType, TargetType>  (outfile);
            break;
        default :
            throw lear::Exception("compute()", "Unspecified format specified");
    }


    if (verbose > 5) {
        cout << "Created formated stream" << endl;
    }
	
    if (do_apca){

	FILE* cov;
	CvMat *poscovmat,*negcovmat,*meancovmat,*negavg,*posavg,*transformvec;

	if ((cov = fopen (outfile1.c_str(), "rb")) == NULL){
	
		int rsample, totalsample;
		// Find featureLength
		for (FileListVector::const_iterator f=posfile.begin();
			f!= posfile.end();++f)
		{
			RawDescIn desc(*f, verbose);
			featureLength = desc.featureLength();
		}
			
		// Perform APCA and obtain the transformation vector for reduction to m dimensions
		TargetType target = 1;
		possize = 0;
		
		CvMat* temp = cvCreateMat(featureLength,1,CV_32FC1);
		CvMat* temp_cov = cvCreateMat(featureLength, featureLength,CV_32FC1);
		posavg = cvCreateMat(featureLength,1,CV_32FC1);
		cvSet(posavg, cvScalarAll(0), NULL);
		poscovmat = cvCreateMat(featureLength,featureLength,CV_32FC1);
		cvSet(poscovmat, cvScalarAll(0), NULL);
		possample = new CvMat*[1];
			
		// Find average positive feature & compute covariance matrix
		for (FileListVector::const_iterator f=posfile.begin();
			f!= posfile.end();++f)
		{
			RawDescIn desc(*f, verbose);
			// Find total number of positive samples
			rsample = desc.featureCount();
			possize+=rsample;
			
			totalsample = rsample;
			while(rsample != 0){
				if (rsample < 20000){
					sizesamples = rsample;
				}
				else{
					sizesamples = 20000;
				}
				possample[0] = cvCreateMat(featureLength,sizesamples,CV_32FC1);
				
				for (int m= 0; m< sizesamples && desc; ++m) {
	    				desc.next(feature);
					// Create OpenCV array of postive samples
					for (int l=featureLength;l--;){
						cvSetReal2D(possample[0], l, m, feature(l));
					}
				}
				rsample-=sizesamples;
				
				cvCalcCovarMatrix((const CvArr**)possample, sizesamples, temp_cov, temp, CV_COVAR_NORMAL + CV_COVAR_COLS);
				cvAdd(temp_cov, poscovmat, poscovmat, NULL);
				cvConvertScale(temp,temp,(float)(sizesamples),0);
				cvAdd(temp, posavg, posavg, NULL);
				cvReleaseMat(&possample[0]);
			}
			
			if (verbose > 2) {
        			cout << "Processed descriptors " << std::setw(7) << sizesamples ;
        			if (verbose > 3) 
            				cout << " from file " << *f;
        			cout << endl;
    			}
			if (maxposvec==0)
				break;
    		}
    		cvReleaseMat(possample);
    		
    		cvConvertScale(posavg,posavg,1/(float)(possize),0);
		cvConvertScale(poscovmat,poscovmat,1/(float)(possize),0);
				
		target = -1;
		negsize=0;
		
		negavg = cvCreateMat(featureLength,1,CV_32FC1);
		cvSet(negavg, cvScalarAll(0), NULL);
		negcovmat = cvCreateMat(featureLength,featureLength,CV_32FC1);
		cvSet(negcovmat, cvScalarAll(0), NULL);
		negsample = new CvMat*[1];
		
		for (FileListVector::const_iterator f=negfile.begin();
			f!= negfile.end();++f)
		{
			RawDescIn desc(*f, verbose);
			rsample = desc.featureCount();
			// Find total number of negative samples
			negsize+=rsample;
			
			totalsample = rsample;
			while(rsample != 0){
				if (rsample < 20000){
					sizesamples = rsample;
				}
				else{
					sizesamples = 20000;
				}
				negsample[0] = cvCreateMat(featureLength,sizesamples,CV_32FC1);
			
				for (int m= 0; m < sizesamples && desc; ++m) {
			    		desc.next(feature);
			    		// Create OpenCV array of negative samples
					for (int l=featureLength;l--;){
						cvSetReal2D(negsample[0], l, m, feature(l));
					}
				}
				rsample-=sizesamples;
				
				cvCalcCovarMatrix((const CvArr**)negsample, sizesamples, temp_cov, temp, CV_COVAR_NORMAL + CV_COVAR_COLS);
				cvAdd(temp_cov, negcovmat, negcovmat, NULL);
				cvConvertScale(temp,temp,(float)(sizesamples),0);
				cvAdd(temp, negavg, negavg, NULL);
				cvReleaseMat(&negsample[0]);
			}
			
			if (verbose > 2) {
        			cout << "Processed descriptors " << std::setw(7) << totalsample ;
        			if (verbose > 3) 
            				cout << " from file " << *f;
        			cout << endl;
    			}
    			
			if (maxnegvec==0)
				break;
		}
		cvReleaseMat(negsample);
		cvReleaseMat(&temp);
		cvReleaseMat(&temp_cov);
		
		cvConvertScale(negavg,negavg,1/(float)(negsize),0);
		cvConvertScale(negcovmat,negcovmat,1/(float)(negsize),0);	

		// Compute covariance matrix of class mean
		meancovmat = cvCreateMat(featureLength,featureLength,CV_32FC1);
		CvMat* posmeancovmat = cvCreateMat(featureLength,featureLength,CV_32FC1);
		CvMat* negmeancovmat = cvCreateMat(featureLength,featureLength,CV_32FC1);
		CvMat* globalavg = cvCreateMat(featureLength,1,CV_32FC1);
		cvAddWeighted(posavg, ((float)possize/(float)(possize+negsize)),negavg, ((float)negsize/(float)(possize+negsize)),0.0, globalavg);
		cvMulTransposed( posavg, posmeancovmat, 0, globalavg);
		cvMulTransposed( negavg, negmeancovmat, 0, globalavg);
		cvAddWeighted(posmeancovmat,((float)possize/(float)(possize+negsize)),negmeancovmat,((float)negsize/(float)(possize+negsize)),0.0, meancovmat);
					
		cvReleaseMat(&globalavg);
		cvReleaseMat(&posmeancovmat);
		cvReleaseMat(&negmeancovmat);

		// Compute overall covariance matrix
		CvMat* totalcovmat = cvCreateMat(featureLength,featureLength,CV_32FC1);
		CvMat* intermediate = cvCreateMat(featureLength,featureLength,CV_32FC1);
		if (wp == 1){
			wp = (float)negsize/(float)(possize+negsize);
		}
		cvAddWeighted(poscovmat, wp, negcovmat, (1-wp), 0.0, intermediate);
		cvAdd(intermediate,meancovmat,totalcovmat,NULL);
			
		cvReleaseMat(&intermediate);
					
		// Compute the eigen-decomposition
		transformvec = cvCreateMat(featureLength,featureLength,CV_32FC1);
		CvMat* eigenvaluemat = cvCreateMat(featureLength,1,CV_32FC1);
		cvSVD(totalcovmat, eigenvaluemat, transformvec, NULL, CV_SVD_MODIFY_A + CV_SVD_U_T);
									
		cvReleaseMat(&totalcovmat);
		cvReleaseMat(&eigenvaluemat);
		
		if ((cov = fopen (outfile1.c_str(), "wb")) == NULL)
			{ throw lear::Exception("compute()", "Model file cannot be opened!");}
			
		fwrite(&possize,sizeof(int),1,cov);
		fwrite(&negsize,sizeof(int),1,cov);
		fwrite(&featureLength,sizeof(int),1,cov);
					
		double elem;
		for (int i=0;i<featureLength;i++){
			elem = cvGetReal1D(posavg,i);
			fwrite(&elem,sizeof(double),1,cov);
			elem = cvGetReal1D(negavg,i);
			fwrite(&elem,sizeof(double),1,cov);
			for (int j=0;j<featureLength;j++){
				elem = cvGetReal2D(poscovmat,i ,j);
				fwrite(&elem,sizeof(double),1,cov);
				elem = cvGetReal2D(negcovmat,i ,j);
				fwrite(&elem,sizeof(double),1,cov);
				elem = cvGetReal2D(meancovmat,i ,j);
				fwrite(&elem,sizeof(double),1,cov);
				elem = cvGetReal2D(transformvec,i,j);
				fwrite(&elem,sizeof(double),1,cov);
			}
		}
		fclose(cov);
	}
	else{
		fread(&possize,sizeof(int),1,cov);
		fread(&negsize,sizeof(int),1,cov);			
		fread(&featureLength, sizeof(int),1,cov);
						
		poscovmat = cvCreateMat(featureLength,featureLength,CV_32FC1);
		negcovmat = cvCreateMat(featureLength,featureLength,CV_32FC1);
		meancovmat = cvCreateMat(featureLength,featureLength,CV_32FC1);
		transformvec = cvCreateMat(featureLength,featureLength,CV_32FC1);
		posavg = cvCreateMat(featureLength,1,CV_32FC1);
		negavg = cvCreateMat(featureLength,1,CV_32FC1);
			
		double elem;

		for (int i=0;i<featureLength;i++){
			fread(&(elem), sizeof(double),1,cov);
			cvSetReal1D(posavg, i, elem);
			fread(&(elem), sizeof(double),1,cov);
			cvSetReal1D(negavg, i, elem);
			for (int j=0;j<featureLength;j++){
				fread(&(elem), sizeof(double),1,cov);
				cvSetReal2D(poscovmat, i, j, elem);
				fread(&(elem), sizeof(double),1,cov);
				cvSetReal2D(negcovmat, i, j, elem);
				fread(&(elem), sizeof(double),1,cov);
				cvSetReal2D(meancovmat, i, j, elem);
				fread(&(elem), sizeof(double),1,cov);
				cvSetReal2D(transformvec, i, j, elem);
			}
		}
		fclose(cov);
	}
	
	if (bootstrap){
		
		int rsample, totalsample;
		CvMat* temp = cvCreateMat(featureLength,1,CV_32FC1);
		CvMat* temp_cov = cvCreateMat(featureLength, featureLength,CV_32FC1);
		
		possample = new CvMat*[1];
		cvConvertScale(posavg,posavg,(float)(possize),0);
		cvConvertScale(poscovmat,poscovmat,(float)(possize),0);
		
		for (FileListVector::const_iterator f=posfile.begin();
			f!= posfile.end();++f)
		{
			RawDescIn desc(*f, verbose);
			// Find total number of positive samples
			rsample = desc.featureCount();
			possize+=rsample;
			
			totalsample = rsample;
			while(rsample != 0){
				if (rsample < 20000){
					sizesamples = rsample;
				}
				else{
					sizesamples = 20000;
				}
				possample[0] = cvCreateMat(featureLength,sizesamples,CV_32FC1);
				
				for (int m= 0; m< sizesamples && desc; ++m) {
	    				desc.next(feature);
					// Create OpenCV array of postive samples
					for (int l=featureLength;l--;){
						cvSetReal2D(possample[0], l, m, feature(l));
					}
				}
				rsample-=sizesamples;
				
				cvCalcCovarMatrix((const CvArr**)possample, sizesamples, temp_cov, temp, CV_COVAR_NORMAL + CV_COVAR_COLS);
				cvAdd(temp_cov, poscovmat, poscovmat, NULL);
				cvConvertScale(temp,temp,(float)(sizesamples),0);
				cvAdd(temp, posavg, posavg, NULL);
				cvReleaseMat(&possample[0]);
			}
						
			if (verbose > 2) {
        			cout << "Processed descriptors " << std::setw(7) << totalsample ;
        			if (verbose > 3) 
            				cout << " from file " << *f;
        			cout << endl;
    			}
			if (maxposvec==0)
				break;
    		}
    		possample[0] = cvCreateMat(1,1,CV_32FC1);
		cvReleaseMat(&possample[0]);
    		cvReleaseMat(possample);
    		
    		cvConvertScale(posavg,posavg,1/(float)(possize),0);
		cvConvertScale(poscovmat,poscovmat,1/(float)(possize),0);	
				
		negsample = new CvMat*[1];
		cvConvertScale(negavg,negavg,(float)(negsize),0);
		cvConvertScale(negcovmat,negcovmat,(float)(negsize),0);
		
		for (FileListVector::const_iterator f=negfile.begin();
			f!= negfile.end();++f)
		{
			RawDescIn desc(*f, verbose);
			rsample = desc.featureCount();
			// Find total number of negative samples
			negsize+=rsample;
			
			totalsample = rsample;
			while(rsample != 0){
				if (rsample < 20000){
					sizesamples = rsample;
				}
				else{
					sizesamples = 20000;
				}
				negsample[0] = cvCreateMat(featureLength,sizesamples,CV_32FC1);
			
				for (int m= 0; m < sizesamples && desc; ++m) {
			    		desc.next(feature);
			    		// Create OpenCV array of negative samples
					for (int l=featureLength;l--;){
						cvSetReal2D(negsample[0], l, m, feature(l));
					}
				}
				rsample-=sizesamples;
				
				cvCalcCovarMatrix((const CvArr**)negsample, sizesamples, temp_cov, temp, CV_COVAR_NORMAL + CV_COVAR_COLS);
				cvAdd(temp_cov, negcovmat, negcovmat, NULL);
				cvConvertScale(temp,temp,(float)(sizesamples),0);
				cvAdd(temp, negavg, negavg, NULL);
				cvReleaseMat(&negsample[0]);
			}
						
			if (verbose > 2) {
        			cout << "Processed descriptors " << std::setw(7) << totalsample ;
        			if (verbose > 3) 
            				cout << " from file " << *f;
        			cout << endl;
    			}
    			
			if (maxnegvec==0)
				break;
		}
		negsample[0] = cvCreateMat(1,1,CV_32FC1);
		cvReleaseMat(&negsample[0]);
		cvReleaseMat(negsample);
		cvReleaseMat(&temp);
		cvReleaseMat(&temp_cov);
		
		cvConvertScale(negavg,negavg,1/(float)(negsize),0);
		cvConvertScale(negcovmat,negcovmat,1/(float)(negsize),0);	
		
		// Compute covariance matrix of class mean
		CvMat* posmeancovmat = cvCreateMat(featureLength,featureLength,CV_32FC1);
		CvMat* negmeancovmat = cvCreateMat(featureLength,featureLength,CV_32FC1);
		CvMat* globalavg = cvCreateMat(featureLength,1,CV_32FC1);
		cvAddWeighted(posavg, ((float)possize/(float)(possize+negsize)),negavg, ((float)negsize/(float)(possize+negsize)),0.0, globalavg);
		cvMulTransposed( posavg, posmeancovmat, 0, globalavg);
		cvMulTransposed( negavg, negmeancovmat, 0, globalavg);
		cvAddWeighted(posmeancovmat,((float)possize/(float)(possize+negsize)),negmeancovmat,((float)negsize/(float)(possize+negsize)),0.0, meancovmat);
					
		cvReleaseMat(&globalavg);
		cvReleaseMat(&posmeancovmat);
		cvReleaseMat(&negmeancovmat);
		
		// Compute overall covariance matrix
		CvMat* totalcovmat = cvCreateMat(featureLength,featureLength,CV_32FC1);
		CvMat* intermediate = cvCreateMat(featureLength,featureLength,CV_32FC1);
		if (wp == 1){
			wp = (float)negsize/(float)(possize+negsize);
		}
		cvAddWeighted(poscovmat, wp, negcovmat, (1-wp), 0.0, intermediate);
		cvAdd(intermediate,meancovmat,totalcovmat,NULL);
			
		cvReleaseMat(&intermediate);
				
		// Compute the eigen-decomposition
		CvMat* eigenvaluemat = cvCreateMat(featureLength,1,CV_32FC1);
		cvSVD(totalcovmat, eigenvaluemat, transformvec, NULL, CV_SVD_MODIFY_A + CV_SVD_U_T);
									
		cvReleaseMat(&totalcovmat);
		cvReleaseMat(&eigenvaluemat);
		
		if ((cov = fopen (outfile1.c_str(), "wb")) == NULL)
			{ throw lear::Exception("compute()", "Model file cannot be opened!");}
			
		fwrite(&possize,sizeof(int),1,cov);
		fwrite(&negsize,sizeof(int),1,cov);
		fwrite(&featureLength,sizeof(int),1,cov);
					
		double elem;
		for (int i=0;i<featureLength;i++){
			elem = cvGetReal1D(posavg,i);
			fwrite(&elem,sizeof(double),1,cov);
			elem = cvGetReal1D(negavg,i);
			fwrite(&elem,sizeof(double),1,cov);
			for (int j=0;j<featureLength;j++){
				elem = cvGetReal2D(poscovmat,i ,j);
				fwrite(&elem,sizeof(double),1,cov);
				elem = cvGetReal2D(negcovmat,i ,j);
				fwrite(&elem,sizeof(double),1,cov);
				elem = cvGetReal2D(meancovmat,i ,j);
				fwrite(&elem,sizeof(double),1,cov);
				elem = cvGetReal2D(transformvec,i,j);
				fwrite(&elem,sizeof(double),1,cov);
			}
		}
		fclose(cov);
	}
				
	CvMat* tmp = cvCreateMat(rednumofdim,featureLength,CV_32FC1);
	for (int i=rednumofdim;i--;){
		for (int j=featureLength;j--;)
			cvSetReal2D(tmp,i,j,cvGetReal2D(transformvec,i,j));
	}

	//Perform dimension reduction on the average vectors and on the covariance matrices of each class
	CvMat* posavgtr = cvCreateMat(rednumofdim,1,CV_32FC1);
	cvGEMM(tmp, posavg, 1, NULL, 0, posavgtr, 0);
	cvReleaseMat(&posavg);

	CvMat* negavgtr = cvCreateMat(rednumofdim,1,CV_32FC1);
	cvGEMM(tmp, negavg, 1, NULL, 0, negavgtr,0);
	cvReleaseMat(&negavg);

	CvMat* tmp1 = cvCreateMat(rednumofdim,featureLength, CV_32FC1); //temp vector for transform
	
	CvMat* poscovmattr = cvCreateMat(rednumofdim,rednumofdim, CV_32FC1);
	cvGEMM(tmp, poscovmat, 1, NULL, 0, tmp1, 0);
	cvGEMM(tmp1, tmp, 1, NULL, 0, poscovmattr, CV_GEMM_B_T);
	cvReleaseMat(&poscovmat);

	CvMat* negcovmattr = cvCreateMat(rednumofdim,rednumofdim, CV_32FC1);
	cvGEMM(tmp, negcovmat, 1, NULL, 0, tmp1, 0);
	cvGEMM(tmp1, tmp, 1, NULL, 0, negcovmattr, CV_GEMM_B_T);
	cvReleaseMat(&negcovmat);
	
	CvMat* meancovmattr = cvCreateMat(rednumofdim,rednumofdim, CV_32FC1);
	cvGEMM(tmp, meancovmat, 1, NULL, 0, tmp1, 0);
	cvGEMM(tmp1, tmp, 1, NULL, 0, meancovmattr, CV_GEMM_B_T);
	cvReleaseMat(&meancovmat);
	
	cvReleaseMat(&tmp1);
	
	FILE* covtr;
	if ((covtr = fopen (outfile2.c_str(), "wb")) == NULL)
		{ throw lear::Exception("compute()", "Model file cannot be opened!");}
	
	fwrite(&possize,sizeof(int),1,covtr);
	fwrite(&negsize,sizeof(int),1,covtr);
	fwrite(&featureLength,sizeof(int),1,covtr);
	fwrite(&rednumofdim,sizeof(int),1,covtr);
	
	double elem;
	
	for (int i=0;i<rednumofdim;i++){
		elem = cvGetReal1D(posavgtr,i);
		fwrite(&elem,sizeof(double),1,covtr);
		elem = cvGetReal1D(negavgtr,i);
		fwrite(&elem,sizeof(double),1,covtr);
		for (int j=0;j<rednumofdim;j++){
			elem = cvGetReal2D(poscovmattr,i ,j);
			fwrite(&elem,sizeof(double),1,covtr);
			elem = cvGetReal2D(negcovmattr,i ,j);
			fwrite(&elem,sizeof(double),1,covtr);
			elem = cvGetReal2D(meancovmattr,i ,j);
			fwrite(&elem,sizeof(double),1,covtr);
		}
		for (int j=0;j<featureLength;j++){
			elem = cvGetReal2D(tmp,i,j);
			fwrite(&elem,sizeof(double),1,covtr);
		}
	}
	fclose(covtr);
	
	cvReleaseMat(&meancovmattr);
	
	//Compute inverse of transformed covariance matrices
	CvMat* poscovmattrinv = cvCreateMat(rednumofdim,rednumofdim, CV_32FC1);
	cvConvertScale(poscovmattr,poscovmattr,alpha1,0.0);
	cvInvert(poscovmattr, poscovmattrinv, CV_SVD_SYM);
	
	CvMat* negcovmattrinv = cvCreateMat(rednumofdim,rednumofdim, CV_32FC1);
	cvConvertScale(negcovmattr,negcovmattr,beta1,0.0);
	cvInvert(negcovmattr, negcovmattrinv, CV_SVD_SYM);
	
	//write to file
	FILE* file;
	if ((file = fopen (outfile.c_str(), "wb")) == NULL)
		{ throw lear::Exception("compute()", "Model file cannot be opened!");}

	fwrite(&featureLength,sizeof(int),1,file);
	fwrite(&rednumofdim,sizeof(int),1,file);
	
	double posdet = cvDet(poscovmattr);
	cout<< "The positive matrix determinant is: "<<posdet<<endl;
	double negdet = cvDet(negcovmattr);
	cout<< "The negative matrix determinant is: "<<negdet<<endl;
	double bias = log(posdet) - log(negdet) + 2*(log(negsize) - log(possize));
	//double bias = log(posdet) - log(negdet);
	cout<< "The threshold is: "<<bias<<endl;

	for (int i=0;i<rednumofdim;i++){
		elem = cvGetReal1D(posavgtr,i);
		fwrite(&elem,sizeof(double),1,file);
		elem = cvGetReal1D(negavgtr,i);
		fwrite(&elem,sizeof(double),1,file);
		for (int j=0;j<rednumofdim;j++){
			elem = cvGetReal2D(poscovmattrinv,i ,j);
			fwrite(&elem,sizeof(double),1,file);
			elem = cvGetReal2D(negcovmattrinv,i ,j);
			fwrite(&elem,sizeof(double),1,file);
		}
		for (int j=0;j<featureLength;j++){
			elem = cvGetReal2D(tmp,i,j);
			fwrite(&elem,sizeof(double),1,file);
		}
	}
	fwrite(&bias,sizeof(double),1,file);
	fclose(file);
	
	cvReleaseMat(&posavgtr);
	cvReleaseMat(&negavgtr);
	cvReleaseMat(&poscovmattr);
	cvReleaseMat(&poscovmattrinv);
	cvReleaseMat(&negcovmattr);
	cvReleaseMat(&negcovmattrinv);
	cvReleaseMat(&tmp);
	cvReleaseMat(&transformvec);
   }
   else { // Do not perform APCA
	TargetType target = 1;
	for (FileListVector::const_iterator f=posfile.begin();
		f!= posfile.end();++f)
	{
		writeData(to,*f,maxposvec,target);
		if (maxposvec==0) break;
	}
	target = -1;
	for (FileListVector::const_iterator f=negfile.begin();
		f!= negfile.end();++f)
	{
		writeData(to,*f,maxnegvec,target);
		if (maxnegvec==0) break;
	}
   }
   delete to;
   if (verbose > 5) {
        cout << "All done " << endl;
   }
}
// }}}

// {{{ main
int main(int argc, char** argv) {
    lear::Cmdline cmdline;
    using namespace lear;

    lear::CustomOption fileformat("out file format options ",BiSVMLight);
    fileformat
        .add("BiSVMLight", BiSVMLight, "Binary custom svmlight format")
        .add("SVMLight",SVMLight, "Default svmlight format")
        .add("BiMatlab", BiMatlab, "Format: <int: ignore this> <int:#features> <int: feat_dim> <float: #features*feat_dim>")
        .add("Matlab", Matlab, "Format: <First line: #features, feat_dim> Next #features line with feat_dim numbers in each line")
        ;

    { // {{{ cmdline
        cmdline.commandName("dump2svmlearn");
        cmdline.version("0.0.1", "Author: Navneet Dalal "
                "(mailto: Navneet.Dalal@inrialpes.fr)");
        cmdline.brief( "Dump descriptor to svm_learn format");

        cmdline.description( "Dump descriptor to svm_learn format");

        using namespace lear;
        cmdline.usageIssues(
    "  'posfile'        positive descriptor file, directory, or a list file.\n"
    "  'negfile'        negative descriptor file, directory, or a list file.\n"
                    );
        cmdline.appendUsageIssues(fileformat.usage());

        cmdline.addOption()
            ("verbose,v",option<int>(&verbose)
                ->defaultValue(0)->minValue(0)->maxValue(9),
                "verbose level")
	    ("do_apca,a",bool_option(&do_apca),
                "perform APCA if true, else do not perform")
            ("maxpos,P",option<int>(&maxposvec)
                ->defaultValue(-1)->minValue(-1),
                "maximum positive vectors, -1 implies no limit")
            ("maxneg,N",option<int>(&maxnegvec)
                ->defaultValue(-1)->minValue(-1),
                "maximum negative vectors, -1 implies no limit")
	    ("positive_weight",option<float>(&wp)
                ->defaultValue(1)->minValue(0.01)->maxValue(1),
                "weight for positive covariance matrix for computation of within class scatter matrix")
	    ("rednumofdim,r",option<int>(&rednumofdim)
                ->defaultValue(3780)->minValue(1),
                "number of dimensions to keep after APCA")
            ("alpha_weight",option<float>(&alpha1)
                ->defaultValue(1)->minValue(0)->maxValue(1),
                "weight for positive covariance matrix in APCA for regularization")
            ("beta_weight",option<float>(&beta1)
                ->defaultValue(1)->minValue(0)->maxValue(1),
                "weight for negative covariance matrix in APCA for regularization")
            ("bootstrap",bool_option(&bootstrap),
                "if true, add hard negatives to covariance matrix")
            ("posfile,p",option< FileListVector >(&posfile),
                    "positive descriptor input file")
            ("negfile,n",option< FileListVector >(&negfile),
                    "negative descriptor input file")
			// select data format
            ("format,f",option<std::string>(&(fileformat.option))
                ->defaultValue(fileformat.defaultOption()),
                "specify out file format")
            ;

            cmdline.addArgument()
                ("outfile",option<std::string>(&outfile),"out file")
		("outfile1",option<std::string>(&outfile1),"out file1")
		("outfile2",option<std::string>(&outfile2),"out file2")
		;
    } // }}}

    int status = cmdline.parse(argc, argv);
    if (status != cmdline.ok) 
        return status;

    formatType = static_cast<FileFormat> (fileformat.check());
    try {
        compute();
    } catch(std::exception& e) {
        cerr << "Caught "<< e.what() << endl;
        return 1;
    } catch(...) {
        cerr << "Caught unknown exception" << endl;
        return 1;
    }
    return 0;
}
// }}}

