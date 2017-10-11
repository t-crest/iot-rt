package tpip

//
// Internet Header Format (RFC 791)
//
//
//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |Version|  IHL  |Type of Service|          Total Length         |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |         Identification        |Flags|      Fragment Offset    |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |  Time to Live |    Protocol   |         Header Checksum       |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                       Source Address                          |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                    Destination Address                        |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                    Options                    |    Padding    |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//


class Ip {

  /**
   * Very simple generation of IP header.
   */

  def doIp(p: Packet, prot: Int, src: Int, dest: Int): Unit = {

    if (p == null) {
      Logger.log("Null pointer in doIP")
    } else if (p.len == 0) {
      Logger.log("Zero length packet")
    } else {
      p.setWord(0, 0x45000000 + p.len) // ip length (header without options)
      p.setWord(4, Ip.getId()) // identification, no fragmentation
      p.setWord(8, (0x20 << 24) + (prot << 16)) // ttl, protocol, clear checksum
      p.setSource(src)
      p.setDest(dest)
      // swap ip addresses
      p.setHalfWord(10, p.checkSum(0, 20))

          // p.llh[6] = 0x0800;

      // send packet to the link layer for the packet
      // p.interf.txQueue.enq(p);	
    }
  }
  
}

object Ip {
  var ipId = 0x12340000

  // TODO: maybe synchronized
  def getId() = {
    ipId += 0x10000
    ipId
  }
}