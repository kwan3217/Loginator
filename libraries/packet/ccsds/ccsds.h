#ifndef ccsds_h
#define ccsds_h

#include "packet.h"

/** Implements a concrete class, which creates packets following the CCSDS
    space packet protocol 133.0-B-1, http://http://public.ccsds.org/publications/archive/133x0b1c2.pdf
    which is itself an update to 102.0-B-5. There are no operational differences between these
    two references, so the code may include references to the older document.

    These packets are called CCSDS packets (pronounced "Space packets" because CCSDS is the same
    length to type but 5 syllables to say).

    Distinctive features of the CCSDS packet are:
*      * Six-byte primary header, including 11-bit apid, 16-bit packet length, and packet sequence counter
*      * Optional secondary header. This header contains the timestamp associated with the packet
*      * Length is up to 65536 payload bytes, or 65542 bytes including all headers
*      * All values are most-significant byte first (big-endian). When bits are transmitted serially, they
         should be transmitted most-significant bit first. This is adhered to even though both the embedded
         controller most often used with this code, and host machines which process packets produced with
         this code, are little-endian machines.
*      * Since the length is encoded at a fixed position in a mandatory header, there are no escape codes
         required. The bad news is that this makes finding a packet start in a sea of bytes difficult. If
         packet stream sync is ever lost, it is possible it will not be recovered.

    This library is oriented to produce rocketometer packets, which have an optional secondary header. When
    present, this header consists solely of the 4-byte timestamp. On a rocketometer or similar device, this
    is the TC0 register at the appropriate time. Timer 0 on rocketometer-type devices is set to tick at 60MHz
    and roll over at 3.6 billion ticks, the exact number of ticks in 1 minute. Therefore the timestamp
    rolls over once per minute. Rocketometer-type devices produce enough packets that it is always possible
    to resolve the rollover ambiguity.

    This library includes support code for a telemetry database. This is called a database, but is kept in
    and OpenDocument spreadsheet (as produced by LibreOffice Calc). I can't document the database in its own
    file, because it is machine-parseable and the documentation would get in the way. I can't document the
    database in the Perl script which parses it, because Doxygen doesn't handle Perl. Therefore I document 
    it here.

    The TelemetryDatabase is a single table, held in a spreadsheet (so that values can be calculated) and
    exportable to a single CSV. Each row describes a single field in a packet. It consists of the following columns:

    -# Apid - Application Process ID, Each different kind of packet has a different apid, and all packets with the same apid have the same structure.
    -# shortName - Name of the packet, must be a valid C++ identifier. This is used as a part of variable names in C++ code generated to write packets.
    -# wrapRobot - set to 'y' if we want to automatically generate code to start and end the packet. This is typically set to 'n' when the header is generated in one routine, most commonly in main.cpp, and the body is generated somewhere else, like a sensor routine.
    -# file_ext - Extension to be used by extractPacket when dumping this packet type
    -# hasTC - C++ expression, which when evaluated in the embedded code in the right context, produces the timestamp to use on this packet. If the packet doesn't require a timestamp, this field is blank
    -# extractor - One of "csv", "source", "dump". Used to control how the extractPacket ground support program creates extracted packet tables.
    -# source - C++ expression, which when evaluated in the embedded code in the right context, produces the value for this field
    -# field - field name to the outside world
    -# type - C++ field type. If the field is an array (char[] is most typical) include the array size right after the
    -# unit - Unit of this field, typically some SI unit for floating-point values, or DN or TC ticks for integer values
    -# description in English

  The program tlmDb.pl is used to parse the TelemetryDatabase (after it is converted to CSV format) and
  write C++ code to be included in the robot code, as well as code for the version of extractPacket to be
*/
class CCSDS: public Packet{
private:
  uint16_t lock_apid;
  static const int n_seq=64;
  uint16_t seq[n_seq];
public:
  CCSDS():lock_apid(0) {};
  /** Start a packet with a given apid in a given buffer
    @param Lbuf circular buffer to put packet into
    @param apid Application Process Identifier (packet ID)
    @param TC optional timestamp. If passed, a secondary header is created (and the appropriate bit in the primary header is set) which
           is four bytes and consists entirely of this timestamp. If the default value is passed, or this parameter is not used, then no secondary header is created.
  */
  virtual bool start(Circular &Lbuf, uint16_t apid, uint32_t TC=0xFFFFFFFF);
  virtual bool finish(uint16_t tag);
  virtual bool fill16(uint16_t in);
  virtual bool fill32(uint32_t in);
  virtual bool fill64(uint64_t in);
  virtual bool fillfp(fp f);
  virtual void forget();
};

struct ccsdsHeader {
  uint16_t apid __attribute__((packed));
  uint16_t seq __attribute__((packed));
  uint16_t length __attribute__((packed));
};

#endif
