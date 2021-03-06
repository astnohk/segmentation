#include "ImageSegmentation.h"




void
SequenceProcessor(const std::string& InputName, const int Start, const int End, const OPTIONS& Options)
{
	ERROR Error("SequenceProcessor");
	const char *Bars = "------------------------------------------------";

	int CurrentFileNum;
	const int History_Max = 4;

	PNM pnm_orig;
	ImgVector<ImgClass::RGB> imgd_in;

	// Calculate least filename length
	size_t InputNameLength =
	    InputName.length() + 1
	    + std::max(count_format_length(InputName), size_t(ceil(log(End) / log(10.0))));

	// Process on the sequence
	for (CurrentFileNum = Start; CurrentFileNum <= End; CurrentFileNum++) {
		std::string InputNameNums;
		// Substitute number to filename
		if (InputName.find_first_of("%") == std::string::npos) {
			InputNameNums = InputName;
		} else {
			char *char_tmp = nullptr;
			try {
				char_tmp = new char[InputNameLength];
			}
			catch (const std::bad_alloc& bad) {
				std::cerr << bad.what() << std::endl;
				Error.Value("char_tmp");
				Error.Malloc();
				goto ExitError;
			}
			sprintf(char_tmp, InputName.c_str(), CurrentFileNum);
			InputNameNums = char_tmp;
			delete[] char_tmp;
			char_tmp = nullptr;
		}
		// Read PNM Files
		if (pnm_orig.read(InputNameNums.c_str()) == PNM_FUNCTION_ERROR) {
			Error.Function("pnm_orig.read");
			Error.File(InputNameNums.c_str());
			Error.FileRead();
			goto ExitError;
		}
		std::cout << "- The input image size is "
		    << pnm_orig.Width() << "x" << pnm_orig.Height() << std::endl
		    << "- and bit depth is " << round(log2(double(pnm_orig.MaxInt()))) << std::endl;

		// Copy color image to ImgVector<ImgClass::RGB> imgd_in
		imgd_in.reset(int(pnm_orig.Width()), int(pnm_orig.Height()));
		for (size_t i = 0; i < imgd_in.size(); i++) {
			if (pnm_orig.isRGB()) {
				imgd_in[i] = ImgClass::RGB(pnm_orig[i], pnm_orig[i + pnm_orig.Size()], pnm_orig[i + 2 * pnm_orig.Size()]);
			} else {
				imgd_in[i] = ImgClass::RGB(pnm_orig[i], pnm_orig[i], pnm_orig[i]);
			}
		}

		// Show Parameters
		std::cout << std::endl
		    << "      --- Parameters ---" << std::endl
		    << "  " << Bars << std::endl;
		if (Options.ResampleSize.width > 0 || Options.ResampleSize.height > 0) {
			std::cout << "  | Resample(" << Options.ResampleMethod << "), 0: z-hold, 1: bicubic" << std::endl;
			std::cout << "  |  "
			    << pnm_orig.Width() << "x" << pnm_orig.Height()
			    << " -> "
			    << Options.ResampleSize.width << "x" << Options.ResampleSize.height
			    << std::endl;
		}

		// Resample
		if (Options.ResampleSize.width > 0 || Options.ResampleSize.height > 0) {
			imgd_in.resample_bicubic(int(Options.ResampleSize.width), int(Options.ResampleSize.height));
		}

		// Segmentation & Output
		ImageSegmentation(imgd_in, pnm_orig.MaxInt(), Options.Mode, InputNameNums);
	}
	return;

// Exit Error
ExitError:
	throw std::logic_error("error: void SequenceProcessor(const std::string&, const int, const int, const OPTIONS&)");
}




// String Library
size_t
count_format_length(const std::string& str)
{
	if (str.length() < 1) {
		return 0;
	}
	std::string::size_type format_start = str.find_first_of("%%");
	if (format_start == std::string::npos) {
		return 0;
	}
	std::string::size_type format_num_start = str.find_first_of("0123456789", format_start);
	std::string::size_type format_num_end = str.find_first_not_of("0123456789", format_num_start) - 1;
	std::string::size_type digits = format_num_end - format_num_start + 1;
	std::string num = str.substr(format_num_start, digits);
	return stoul(num);
}

