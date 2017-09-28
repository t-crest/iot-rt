package tpip

/**
 * A simple example app for the TCP/IP stack
 */
object App extends App {

  println("Hello from the real-time TCP/IP app")
  
  val tpip = new Tpip(args(0))
  
  // This is our simulated periodic loop.
  // It is not correct as we wait a constant time and not for the next period.
  var i = 0
  while (true) {
//    println("ping")
    Thread.sleep(1000)
    tpip.run()
    i += 1
    if (i == -1) {
      val p = Packet.freePool.deq
      p.buf(0) = 0xab.toByte
      p.buf(1) = 0xcd.toByte
      p.len = 2
      tpip.ll.txQueue.enq(p)
    }
  }
}
