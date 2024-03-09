#include "helpers.h"

#include "./base64.h"
#include "./QrToPng.h"
#include "./certusPDF.h"

#include "../libs/PDF-Writer-master/PDFWriter/InputByteArrayStream.h"

#include <algorithm>

#include <ft2build.h>

#include FT_TRUETYPE_TABLES_H 


bool stringIsAscii7(const std::string& s)
{
    for (auto c : s)
        if (static_cast<unsigned char>(c) > 127)
            return false;

    return true;
}

std::string stringTrimEdges(std::string& s)
{
	s.erase( s.begin(), std::find_if( s.begin(), s.end(), [](unsigned char ch) { return not std::isspace(ch); } ) );

	s.erase( std::find_if( s.rbegin(), s.rend(), [](unsigned char ch) { return not std::isspace(ch); } ).base(), s.end() );

	return s;
}

std::size_t stringFindNthOccuranceIndex(const std::string& Input, const char Character, const std::size_t Occurance)
{
    if (Occurance)
        for (std::size_t index = 0, occurances = 0; index < Input.size(); ++index)
            if (Input[index] == Character and (++occurances == Occurance))
                return index;

    return std::string::npos;
}

std::vector<std::string> stringSplit(const std::string& str, const char strDelimiter)
{
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;

    while (std::getline(iss, token, strDelimiter))
        tokens.push_back(token);

    return tokens;
}

std::string stringExtractHyperlink(std::string& s)
{
    const std::size_t sPosition = ('S' == s[1]) ? 1 : (('S' == s[2]) ? 2 : 0);

    if (sPosition not_eq 0 and s[sPosition + 1] == '\"')
    {
        const std::size_t rightQuotesPosition = s.find('\"', sPosition + 2);

        if (rightQuotesPosition not_eq std::string::npos)
        {
            auto hyperlink = s.substr(sPosition + 2, rightQuotesPosition - sPosition - 2);

            s = s.substr(0, sPosition) + s.substr(rightQuotesPosition + 1, s.size());

            return hyperlink;
        }
    }

    return {};
}

double sGetUnderlineThicknessFactor(PDFUsedFont* font)
{
    const auto inFTWrapper = font->GetFreeTypeFont();

    void* tableInfo = FT_Get_Sfnt_Table(*inFTWrapper, ft_sfnt_post);

    if(tableInfo)
        return ((TT_Postscript*) tableInfo)->underlineThickness * 1.0 / (*inFTWrapper)->units_per_EM;

    else
        return 0.05;
}

double sGetUnderlinePositionFactor(PDFUsedFont* font)
{
    const auto inFTWrapper = font->GetFreeTypeFont();

    void* tableInfo = FT_Get_Sfnt_Table(*inFTWrapper, ft_sfnt_post);

    if (tableInfo)
        return ((TT_Postscript*) tableInfo)->underlinePosition * 1.0 / (*inFTWrapper)->units_per_EM;

    else
        return -0.15;
}

PDFFormXObject* base64StringToImage
(
    PDFWriter& pdfWriter, 
    const std::string imageType, 
    const std::string base64Input
    )
{
    std::vector<unsigned char> decodedData = base64_decode(base64Input);

    auto byteImage = InputByteArrayStream(decodedData.data(), static_cast<long long>(decodedData.size()));

    if (imageType == "JPG")
        return pdfWriter.CreateFormXObjectFromJPGStream(&byteImage);

    else if (imageType == "PNG")
        return pdfWriter.CreateFormXObjectFromPNGStream(&byteImage);

    return nullptr;
}

PDFFormXObject* stringToQRCodePNGImage
(
    PDFWriter& pdfWriter, 
    const std::string& input, 
    const int imageSize
    )
{
    auto temp = QrToPng("", imageSize, 3, input, false, qrcodegen::QrCode::Ecc::MEDIUM);

    std::vector<unsigned char> outVector;
    outVector.resize(certusPDF::maxImageSize);
                            
    temp.writeToPNG(outVector);

    auto byteImage = InputByteArrayStream(outVector.data(), static_cast<long long>(outVector.size()));

    return pdfWriter.CreateFormXObjectFromPNGStream(&byteImage);
}