package rtapi

object RtThread {

  var rtt = List[RtThread]()

  def startMission(): Unit = {
    val start = System.currentTimeMillis() + 300
    rtt.foreach { x => x.next = start }
    rtt.foreach { x => x.thread.start() }
  }
}

abstract class RtThread(period: Int) extends Runnable {

  var next = System.currentTimeMillis() + period
  
  val thread = new Thread(this)
  RtThread.rtt = this :: RtThread.rtt

  def waitForNextPeriod(): Unit = {
    next += period
    val sleep = next - System.currentTimeMillis() - 5
    if (sleep > 0) Thread.sleep(sleep)
    while (System.currentTimeMillis() - next < 0) {
      ; // busy wait
    }
  }
}