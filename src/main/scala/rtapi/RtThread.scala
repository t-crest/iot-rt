package rtapi

object RtThread {

  // TODO have a list and start all threads at one point
  //  val rtt

  def startThreads(): Unit = {

  }
}

abstract class RtThread(period: Int) extends Runnable {

  var next = System.currentTimeMillis() + period

  val thread = new Thread(this)

  thread.start()

  def waitForNextPeriod(): Unit = {
    next += period
    val sleep = next - System.currentTimeMillis() - 5
    if (sleep > 0) Thread.sleep(sleep)
    while (System.currentTimeMillis() - next < 0) {
      ; // busy wait
    }
  }
}