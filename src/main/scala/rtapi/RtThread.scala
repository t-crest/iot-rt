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

  var next = 0L // set on mission start
  
  def runIt(): Unit = {
    this.run()
  }
  
  val thread = new Thread(new Runnable {
    def run() = {
      waitForNextPeriod()
      runIt()
    }
  })
  
  RtThread.rtt = this :: RtThread.rtt

  def waitForNextPeriod(): Unit = {
    val sleep = next - System.currentTimeMillis() - 5
    if (sleep > 0) Thread.sleep(sleep)
    while (System.currentTimeMillis() - next < 0) {
      ; // busy wait
    }
    next += period
  }
}