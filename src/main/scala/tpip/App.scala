package tpip

import rtapi._

class PeriodicApp(period: Int) extends RtThread(period) {

  println("Hello from the real-time TCP/IP app")

  val tpip = new Tpip()
  var i = 0

  def run(): Unit = {
    while (true) {
      tpip.run()
      i += 1
      if (i == 3) {
        println("Send a ping")
        val p = tpip.ll.txChannel.freePool.deq
        p.buf(0) = 0xab.toByte
        p.buf(1) = 0xcd.toByte
        p.len = 2
        tpip.ll.txChannel.queue.enq(p)
      }
      waitForNextPeriod()
    }
  }
}

/**
 * A simple example app for the TCP/IP stack
 */
object App extends App {

  new PeriodicApp(1000)
  
  RtThread.startMission()
}
