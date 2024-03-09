#include <vector>

#include "../libs/PDF-Writer-master/PDFWriter/PDFWriter.h"
#include "../libs/PDF-Writer-master/PDFWriter/PDFUsedFont.h"
#include "../libs/PDF-Writer-master/PDFWriter/PDFFormXObject.h"


bool stringIsAscii7(const std::string& s);

std::string stringTrimEdges(std::string& s);

std::size_t stringFindNthOccuranceIndex(const std::string& Input, const char Character, const std::size_t Occurance = 1);

std::vector<std::string> stringSplit(const std::string& str, const char strDelimiter = ' ');

std::string stringExtractHyperlink(std::string& s);

double sGetUnderlineThicknessFactor(PDFUsedFont* font);

double sGetUnderlinePositionFactor(PDFUsedFont* font);

PDFFormXObject* base64StringToImage(PDFWriter& pdfWriter, const std::string imageType, const std::string base64Input);

PDFFormXObject* stringToQRCodePNGImage (PDFWriter& pdfWriter, const std::string& input, const int imageSize);