#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkN4BiasFieldCorrectionImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"

// Bias Correction: N4
// Denoising:Anisotropic Diffusion/Gaussian filter should be used

typedef itk::Image<float, 3>  ImageType;
typedef itk::Image<float, 3>  OutImageType;
typedef itk::ImageFileReader< ImageType > ReaderType;
typedef itk::ImageFileWriter< OutImageType > WriterType;
typedef itk::N4BiasFieldCorrectionImageFilter <ImageType, OutImageType> BiasFilterType;
typedef itk::GradientAnisotropicDiffusionImageFilter <ImageType, OutImageType> DiffusionFilterType;

int main(int argc, char * argv[])
{
	ReaderType::Pointer reader1 = ReaderType::New();
	//ReaderType::Pointer reader2 = ReaderType::New();
	WriterType::Pointer writer = WriterType::New();
	

	//Parameters
	reader1->SetFileName(argv[1]);// this is for command line input1 clean
	//reader2->SetFileName(argv[2]);// this is for command line input2 unclean image
	writer->SetFileName(argv[2]);// this is the output image preprocessed

	
	//Pipeline
	try
	{
		reader1->Update();
		//reader2->Update();
	}
	catch (itk::ExceptionObject &err)
	{
		std::cout << "Problems reading input image" << std::endl;
		std::cerr << "ExceptionObject caught!" << std::endl;
		std::cerr << err << std::endl;
		return EXIT_FAILURE;
	}


	//Get image specs
	ImageType::SpacingType spacing = reader1->GetOutput()->GetSpacing();
	ImageType::PointType origin = reader1->GetOutput()->GetOrigin();
	ImageType::DirectionType direction = reader1->GetOutput()->GetDirection();
	ImageType::SizeType  size = reader1->GetOutput()->GetRequestedRegion().GetSize();
	int pRow, pCol, pSli;
	pRow = size[0];
	pCol = size[1];
	pSli = size[2];
	ImageType::RegionType region;
	region.SetSize(size);

	//Bias Correction of the unclean Image
	BiasFilterType::Pointer correctorFilter = BiasFilterType::New();
	correctorFilter->SetInput(reader1->GetOutput());
	correctorFilter->Update();

	

	//Denoising of the Unclean Image
	DiffusionFilterType::Pointer denoisingFilter = DiffusionFilterType::New();
	denoisingFilter->SetInput(correctorFilter->GetOutput());
	denoisingFilter->SetNumberOfIterations(5);
	denoisingFilter->SetTimeStep(0.07);// try 0.625, 0.0867 
	denoisingFilter->SetConductanceParameter(2);// try values= 4-10
	denoisingFilter->UseImageSpacingOn();
	denoisingFilter->Update();

	/*
	AddImageFilterType::Pointer addFilter = AddImageFilterType::New ();
	addFilter->SetInput1(reader1->GetOutput());
	addFilter->SetInput2(reader2->GetOutput());
	addFilter->Update();
	

	HistogramFiltertype::Pointer histo = HistogramFiltertype::New();
	histo->ThresholdAtMeanIntensityOn();
	histo->SetInput(denoisingFilter->GetOutput());
	histo->SetReferenceImage(reader2->GetOutput());
	histo->SetNumberOfHistogramLevels(100); 
	histo->SetNumberOfMatchPoints(15); 
	histo->Update();
	*/
	writer->SetInput(denoisingFilter->GetOutput());

	try
	{
		writer->Update();
	}
	catch (itk::ExceptionObject & err)
	{
		std::cout << "ExceptionObject caught !" << std::endl;
		std::cout << err << std::endl;
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;


}
//Next after getting the output images, open fiji and change from 32 bit to 8 bit and later
// go calculator put the image1 and image2 and subtract and apply- histogram is generated
// do this for other inputs too
//boykov filter