package tpip

abstract class LinkLayer extends Runnable {
  
  val MAX_PACKETS = 300

  val rxChannel = new PacketChannel(MAX_PACKETS)
  val txChannel = new PacketChannel(MAX_PACKETS)
}