#include <istream>
#include <cstdint>

namespace pdal
{

namespace connector
{
class Connector;
}

// An interface that holds an std::istream for a reader to read from.
class ReaderInput
{
public:
    ReaderInput()
    {}

    virtual ~ReaderInput()
    {}

    virtual void prepareRangeForReading(uint64_t offset, int32_t size)
    {
        // Default implementation is to do nothing -
        // only implementations that need to ready individual ranges need to override.
    }

    std::istream *m_istream;
};

// ReaderInput that uses Utils::openFile to open the input as a local file
// (a local TempFile if the filename is remote).
class LocalFileReaderInput : public ReaderInput
{
public:
    LocalFileReaderInput(const std::string& filename);
    virtual ~LocalFileReaderInput();
};

// ReaderInput that manages an input stream backed by one requested range at a time,
// and fetches requested ranges using a connector::Connector.
class BufferedRangeReaderInput : public ReaderInput
{
public:
    BufferedRangeReaderInput(const std::string& filename);
    virtual ~BufferedRangeReaderInput();
    virtual void prepareRangeForReading(uint64_t offset, int32_t size);

private:
    connector::Connector *m_connector;
};

} // namespace pdal
