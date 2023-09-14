
#include "ReaderInput.hpp"
#include "connector/Connector.hpp"

#include <pdal/PDALUtils.hpp>

namespace pdal
{

namespace
{

// Adapts a vector<char> representing some part of a file to std::istream
class BufferedRangeStream: public virtual std::streambuf, public std::istream
{
public:
    BufferedRangeStream()
        : std::istream(static_cast<std::streambuf*>(this))
    {}

    void setRange(uint64_t offset, std::vector<char> range)
    {
        m_offset = offset;
        range.swap(m_range);
        setg(&*m_range.begin(), &*m_range.begin(), &*m_range.end());
    }

    std::istream::pos_type seekpos(std::istream::pos_type sp,
        std::ios_base::openmode which) override
    {
        return seekoff(sp - std::istream::pos_type(std::istream::off_type(0)),
            std::ios_base::beg, which);
    }

    std::istream::pos_type seekoff(std::istream::off_type off,
        std::ios_base::seekdir dir,
        std::ios_base::openmode which = std::ios_base::in) override
    {
        if (dir == std::ios_base::cur)
            gbump(off);
        else if (dir == std::ios_base::beg)
            setg(eback(), eback() + off - m_offset, egptr());
        return m_offset + gptr() - eback();
    }

    std::vector<char> m_range;
    uint64_t m_offset;
};

} // namespace

LocalFileReaderInput::LocalFileReaderInput(const std::string& filename)
{
    m_istream = Utils::openFile(filename);
}

LocalFileReaderInput::~LocalFileReaderInput()
{
    if (m_istream)
        Utils::closeFile(m_istream);
}

BufferedRangeReaderInput::BufferedRangeReaderInput(const std::string& filename)
{
    StringMap headers;
    StringMap query;
    m_connector = new connector::Connector(filename, headers, query);
    m_istream = new BufferedRangeStream();
}

BufferedRangeReaderInput::~BufferedRangeReaderInput()
{
    delete m_istream;
    delete m_connector;
}

void BufferedRangeReaderInput::prepareRangeForReading(uint64_t offset, int32_t size)
{
    BufferedRangeStream* brs = reinterpret_cast<BufferedRangeStream*>(m_istream);
    if (offset < brs->m_offset || offset + size > brs->m_offset + brs->m_range.size())
    {
        // Requested range not inside current range - prepare new buffered range.
        brs->setRange(offset, m_connector->getBinary(offset, size));
    }
}

} // namespace pdal
