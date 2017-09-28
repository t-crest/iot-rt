package tpip

class Ip {

  /**
   * Very simple generation of IP header. Just swap source and destination.
   * Maybe used by ICMP and TCP.
   */

  def doIp(p: Packet, prot: Int): Unit = {

    def setWord(pos: Int, word: Int): Unit = {
      p.buf(pos) = (word >>> 24).toByte
      p.buf(pos + 1) = (word >>> 16).toByte
      p.buf(pos + 2) = (word >>> 8).toByte
      p.buf(pos + 3) = (word).toByte
    }

    def getWord(pos: Int): Int = {
      ((p.buf(pos) & 0xff) << 24) +
        ((p.buf(pos + 1) & 0xff) << 16) +
        ((p.buf(pos + 2) & 0xff) << 8) +
        ((p.buf(pos + 3) & 0xff))
    }

    if (p == null) {
      println("Null pointer in doIP")
    } else if (p.len == 0) {
      println("Zero length packet")
    } else {
      setWord(0, 0x45000000 + p.len) // ip length (header without options)
      setWord(4, Ip.getId()) // identification, no fragmentation
      setWord(8, (0x20 << 24) + (prot << 16)) // ttl, protocol, clear checksum
      // swap ip addresses
      val adr = getWord(12)
      setWord(12, getWord(16))
      setWord(16, 12)
      // TODO checksum
      // buf[2] |= Ip.chkSum(buf, 0, 20);

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