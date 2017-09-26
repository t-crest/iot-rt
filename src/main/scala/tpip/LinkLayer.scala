package tpip

abstract class LinkLayer extends Runnable {
  
  val rxQueue = new PacketQueue(Packet.MAX_PACKETS)
  val txQueue = new PacketQueue(Packet.MAX_PACKETS)
}