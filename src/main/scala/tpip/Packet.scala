package tpip

class Packet {

  val MAX_LEN = 1000

  val buf = new Array[Byte](MAX_LEN)
  var len = 0

  def set(b: Array[Byte]): Unit = {

    len = if (b.length > buf.length) buf.length else b.length

    for (i <- 0 until len) {
      buf(i) = b(i)
    }
  }
  
  def checkSum(off: Int, cnt: Int): Int = {
    
    if ((cnt & 0x01) != 0) {
      buf(cnt) = 0
    }
    var sum = 0
    for (i <- 0 until cnt/2) {
      sum += ((buf(i*2)<<8) + buf(i*2+1) & 0xff) & 0xffff
    }
   while ((sum >> 16) != 0)
			sum = (sum & 0xffff) + (sum >> 16)
	
		(~sum) & 0xffff
  }
}

object Packet {

  val MAX_PACKETS = 3
  val freePool = new PacketQueue(MAX_PACKETS)

  for (i <- 0 until MAX_PACKETS) freePool.enq(new Packet())
}
