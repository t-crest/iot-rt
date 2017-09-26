package iotrt

class Packet {
  
  val MAX_LEN = 1000

  val buf = new Array[Byte](MAX_LEN)
  val len = 0
}

object Packet {
  
  val MAX_PACKETS = 10
  val freePool = new Array[Packet](MAX_PACKETS)
  
  for (i <- 0 until freePool.length) {
    freePool(i) = new Packet()
  }
}
