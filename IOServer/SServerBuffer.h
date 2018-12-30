#ifndef _INC_SSERVERBUFFER_
#define _INC_SSERVERBUFFER_
//////////////////////////////////////////////////////////////////////////
class SServerBuffer
{
public:
	SServerBuffer()
	{
		m_uBufferLen = 0;
		m_pBuffer = NULL;
		m_uDataLength = 0;
		m_uReadOffset = 0;
	}
	~SServerBuffer()
	{
		FreeBuffer();
	}

public:
	void AllocBuffer(size_t _uSize)
	{
		FreeBuffer();
		m_pBuffer = (char*)malloc(_uSize);
		m_uBufferLen = _uSize;
	}
	void FreeBuffer()
	{
		if(NULL != m_pBuffer)
		{
			free(m_pBuffer);
			m_pBuffer = NULL;
			m_uBufferLen = 0;
			m_uReadOffset = 0;
		}
	}
	//	double the buffer size
	void ReallocBuffer()
	{
		m_uBufferLen *= 2;
		m_pBuffer = (char*)realloc(m_pBuffer, m_uBufferLen);

		if(NULL == m_pBuffer)
		{
			m_uBufferLen = 0;
			m_uDataLength = 0;
			m_uReadOffset = 0;
		}
	}

	size_t Write(const char* _pData, size_t _uLen)
	{
		if(m_uBufferLen - m_uDataLength < _uLen)
		{
			ReallocBuffer();
		}

		if(NULL == m_pBuffer)
		{
			return 0;
		}

		memcpy(m_pBuffer + m_uDataLength, _pData, _uLen);
		m_uDataLength += _uLen;

		return _uLen;
	}

	int Read(char* _pBuffer, size_t _uLen)
	{
		if(_uLen > m_uDataLength - m_uReadOffset)
		{
			_uLen = m_uDataLength - m_uReadOffset;
		}

		if(NULL != _pBuffer)
		{
			memcpy(_pBuffer, m_pBuffer + m_uReadOffset, _uLen);
		}

		//	offset
		m_uReadOffset += _uLen;

		return _uLen;
	}

	void IncDataLength(size_t _nDiff)
	{
		m_uDataLength += _nDiff;
	}

	size_t GetBufferSize()
	{
		return m_uBufferLen;
	}

	size_t GetReadableSize()
	{
		return m_uDataLength - m_uReadOffset;
	}

	void Rewind()
	{
		m_uReadOffset = 0;
	}

	char* GetFreeBufferPtr()
	{
		return m_pBuffer + m_uDataLength;
	}

	char* GetReadableBufferPtr()
	{
		return m_pBuffer + m_uReadOffset;
	}

	void Reset()
	{
		m_uDataLength = 0;
		m_uReadOffset = 0;
	}

	size_t GetAvailableSize()
	{
		return m_uBufferLen - m_uDataLength;
	}

	void SetDataLength(unsigned int _uLength)
	{
		m_uDataLength = _uLength;
	}

private:
	char* m_pBuffer;
	size_t m_uBufferLen;
	size_t m_uDataLength;
	size_t m_uReadOffset;
};
//////////////////////////////////////////////////////////////////////////
#endif