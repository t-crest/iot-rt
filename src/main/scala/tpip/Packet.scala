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
}

object Packet {

  val MAX_PACKETS = 3
  val freePool = new PacketQueue(MAX_PACKETS)

  for (i <- 0 until MAX_PACKETS) freePool.enq(new Packet())
}
