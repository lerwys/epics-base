#ifndef CFSTREAM_H
#define CFSTREAM_H

#include <cstdio>

#include <cstring>
#include <istream>
#include <ostream>
#include <vector>

class cfile_streambuf : public std::streambuf
{
public:
    cfile_streambuf(size_t bufsize=1024);
    explicit cfile_streambuf(FILE *fp, size_t bufsize=1024);
    ~cfile_streambuf();

    void set(FILE*);
    //inline FILE* get() const {return fp;}
    void close();

    virtual int_type underflow();
    virtual int_type overflow(int_type=traits_type::eof());
    virtual int sync();

private:
    cfile_streambuf(const cfile_streambuf&);
    cfile_streambuf& operator=(const cfile_streambuf&);

    int doFlush();

    FILE *fp;
    /* The read buffer:
     * readbuf.front() -> eback()
     * &readbuf[n]     -> gptr() n in [0, size)
     * readbuf.back()()-> egptr()
     */
    std::vector<char_type> readbuf, writebuf;
    char_type *endreadbuf, *endwritebuf;
};

/** An I/O stream backed by a C FILE*
 */
class iocfstream : public std::iostream
{
    cfile_streambuf filebuf;
public:
    explicit iocfstream(size_t bufsize=1024)
        :std::iostream(&filebuf), filebuf(bufsize) {}
    explicit iocfstream(FILE *fp, size_t bufsize=1024)
        :std::iostream(&filebuf), filebuf(fp, bufsize) {}

    inline void setFile(FILE *fp) {filebuf.set(fp);}
};

#endif // CFSTREAM_H
