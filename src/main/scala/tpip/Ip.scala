package tpip

class Ip {

  /**
   * Very simple generation of IP header. Just swap source and destination.
   * Maybe used by ICMP and TCP.
   */

  def doIp(p: Packet, prot: Int): Unit = {

    if (p == null) {
      Logger.log("Null pointer in doIP")
    } else if (p.len == 0) {
      Logger.log("Zero length packet")
    } else {
      p.setWord(0, 0x45000000 + p.len) // ip length (header without options)
      p.setWord(4, Ip.getId()) // identification, no fragmentation
      p.setWord(8, (0x20 << 24) + (prot << 16)) // ttl, protocol, clear checksum
      // swap ip addresses
      val adr = p.getWord(12)
      p.setWord(12, p.getWord(16))
      p.setWord(16, 12)
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