#include "pdfBuffer.h"

pdfBuffer::pdfBuffer(const IOBasicTypes::LongBufferSizeType size)
{
	_buffer.reserve(size);
}

IOBasicTypes::LongFilePositionType pdfBuffer::GetCurrentPosition(void)
{
	return _position;
}

IOBasicTypes::LongBufferSizeType pdfBuffer::Write(const IOBasicTypes::Byte* inBuffer, IOBasicTypes::LongBufferSizeType inSize)
{
	if (inSize <= _buffer.size() - _position)
	{
		std::memcpy(_buffer.data() + _position, inBuffer, inSize);

		_position += inSize;
	}

	else if (inSize <= MAX_BUFFER_SIZE - _position)
	{
		_buffer.resize(_position + inSize);

		std::memcpy(_buffer.data() + _position, inBuffer, inSize);

		_position += inSize;
	}

	else
		return 0;

	return inSize;
}

std::size_t pdfBuffer::maxSize(void)
{
	return _buffer.size();
}

std::size_t pdfBuffer::size(void)
{
	return _position;
}

void pdfBuffer::copy(IOBasicTypes::Byte* outBuffer, IOBasicTypes::LongBufferSizeType outSize)
{
	if (outBuffer != nullptr)
	{
		if (outSize == MAX_BUFFER_SIZE)
			std::memcpy(outBuffer, _buffer.data(), _position);
	
		else if (outSize < _position)
			std::memcpy(outBuffer, _buffer.data(), outSize);
	}
}

IOBasicTypes::Byte* pdfBuffer::data(void)
{
	return _buffer.data();
}