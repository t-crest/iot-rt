package tpip

class PacketChannel(n: Int) {
  
  
  val freePool = new PacketQueue(n)
  val queue = new PacketQueue(n)

  for (i <- 0 until n) freePool.enq(new Packet())
}