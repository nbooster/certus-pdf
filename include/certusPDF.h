#pragma once

#include <vector>

namespace certusPDF
{
	constexpr char delimiter = ',';
	constexpr std::size_t maxImageSize = 100'000;
	constexpr std::size_t textDelimiterStartPosition = 6;
	constexpr std::size_t pointsDelimiterStartPosition = 4;

	std::pair<bool, std::string> createPDF
	(
	    const std::vector<std::vector<std::string>>& inputPages,
	    const std::string& fontFilePath,
	    const std::string& pdfFilePath,
	    const int dimX = 595,
	    const int dimY = 842
	);
}
