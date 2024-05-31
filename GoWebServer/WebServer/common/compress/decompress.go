package myzip

import (
	"bufio"
	"compress/flate"
	"errors"
	"hash/crc32"
	"io"
	"time"
)

var (
	ErrChecksum = errors.New("gzip: invalid checksum")
	ErrHeader   = errors.New("gzip: invalid header")
)

func noEOF(err error) error {
	if err == io.EOF {
		return io.ErrUnexpectedEOF
	}
	return err
}

type Reader struct {
	Header       // valid after NewReader or Reader.Reset
	r            flate.Reader
	decompressor io.ReadCloser
	digest       uint32 // CRC-32, IEEE polynomial (section 8)
	size         uint32 // Uncompressed size (section 2.3.1)
	buf          [512]byte
	err          error
	multistream  bool
}

func NewReader(r io.Reader) (*Reader, error) {
	z := new(Reader)
	if err := z.Reset(r); err != nil {
		return nil, err
	}
	return z, nil
}

func (z *Reader) Reset(r io.Reader) error {
	*z = Reader{
		decompressor: z.decompressor,
		multistream:  true,
	}
	if rr, ok := r.(flate.Reader); ok {
		z.r = rr
	} else {
		z.r = bufio.NewReader(r)
	}
	z.Header, z.err = z.readHeader()
	return z.err
}

func (z *Reader) readString() (string, error) {
	var err error
	needConv := false
	for i := 0; ; i++ {
		if i >= len(z.buf) {
			return "", ErrHeader
		}
		z.buf[i], err = z.r.ReadByte()
		if err != nil {
			return "", err
		}
		if z.buf[i] > 0x7f {
			needConv = true
		}
		if z.buf[i] == 0 {
			// Digest covers the NUL terminator.
			z.digest = crc32.Update(z.digest, crc32.IEEETable, z.buf[:i+1])

			// Strings are ISO 8859-1, Latin-1 (RFC 1952, section 2.3.1).
			if needConv {
				s := make([]rune, 0, i)
				for _, v := range z.buf[:i] {
					s = append(s, rune(v))
				}
				return string(s), nil
			}
			return string(z.buf[:i]), nil
		}
	}
}

func (z *Reader) readHeader() (hdr Header, err error) {
	if _, err = io.ReadFull(z.r, z.buf[:10]); err != nil {
		// RFC 1952, section 2.2, says the following:
		//	A gzip file consists of a series of "members" (compressed data sets).
		//
		// Other than this, the specification does not clarify whether a
		// "series" is defined as "one or more" or "zero or more". To err on the
		// side of caution, Go interprets this to mean "zero or more".
		// Thus, it is okay to return io.EOF here.
		return hdr, err
	}
	if z.buf[0] != gzipID1 || z.buf[1] != gzipID2 || z.buf[2] != gzipDeflate {
		return hdr, ErrHeader
	}
	flg := z.buf[3]
	if t := int64(le.Uint32(z.buf[4:8])); t > 0 {
		// Section 2.3.1, the zero value for MTIME means that the
		// modified time is not set.
		hdr.ModTime = time.Unix(t, 0)
	}
	// z.buf[8] is XFL and is currently ignored.
	hdr.OS = z.buf[9]
	z.digest = crc32.ChecksumIEEE(z.buf[:10])

	if flg&flagExtra != 0 {
		if _, err = io.ReadFull(z.r, z.buf[:2]); err != nil {
			return hdr, noEOF(err)
		}
		z.digest = crc32.Update(z.digest, crc32.IEEETable, z.buf[:2])
		data := make([]byte, le.Uint16(z.buf[:2]))
		if _, err = io.ReadFull(z.r, data); err != nil {
			return hdr, noEOF(err)
		}
		z.digest = crc32.Update(z.digest, crc32.IEEETable, data)
		hdr.Extra = data
	}

	var s string
	if flg&flagName != 0 {
		if s, err = z.readString(); err != nil {
			return hdr, noEOF(err)
		}
		hdr.Name = s
	}

	if flg&flagComment != 0 {
		if s, err = z.readString(); err != nil {
			return hdr, noEOF(err)
		}
		hdr.Comment = s
	}

	if flg&flagHdrCrc != 0 {
		if _, err = io.ReadFull(z.r, z.buf[:2]); err != nil {
			return hdr, noEOF(err)
		}
		digest := le.Uint16(z.buf[:2])
		if digest != uint16(z.digest) {
			return hdr, ErrHeader
		}
	}

	z.digest = 0
	if z.decompressor == nil {
		z.decompressor = flate.NewReader(z.r)
	} else {
		z.decompressor.(flate.Resetter).Reset(z.r, nil)
	}
	return hdr, nil
}

func (z *Reader) Read(p []byte) (n int, err error) {
	if z.err != nil {
		return 0, z.err
	}

	for n == 0 {
		n, z.err = z.decompressor.Read(p)
		z.digest = crc32.Update(z.digest, crc32.IEEETable, p[:n])
		z.size += uint32(n)
		if z.err != io.EOF {
			// In the normal case we return here.
			return n, z.err
		}

		// Finished file; check checksum and size.
		if _, err := io.ReadFull(z.r, z.buf[:8]); err != nil {
			z.err = noEOF(err)
			return n, z.err
		}
		digest := le.Uint32(z.buf[:4])
		size := le.Uint32(z.buf[4:8])
		if digest != z.digest || size != z.size {
			z.err = ErrChecksum
			return n, z.err
		}
		z.digest, z.size = 0, 0

		// File is ok; check if there is another.
		if !z.multistream {
			return n, io.EOF
		}
		z.err = nil // Remove io.EOF

		if _, z.err = z.readHeader(); z.err != nil {
			return n, z.err
		}
	}

	return n, nil
}

func (z *Reader) Close() error { return z.decompressor.Close() }
