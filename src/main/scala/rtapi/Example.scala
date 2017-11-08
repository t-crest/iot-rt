/**
 * Example of periodic threads for our simple RtThread runtime.
 */

package rtapi

class Ping(period: Int) extends RtThread(period) {
  
  def run(): Unit = {
    // wait till start
    waitForNextPeriod()
    
    // the endless loop
    while (true) {
      println("ping")
      waitForNextPeriod()
    }
  }
}

class Pong(period: Int) extends RtThread(period) {
  
  def run(): Unit = {
    // wait till start
    waitForNextPeriod()
    
    // the endless loop
    while (true) {
      println("pong")
      waitForNextPeriod()
    }
  }
}
object Example extends App {
  
  val pi = new Ping(1000)
  val po = new Pong(3000)
  
  RtThread.startMission()  
}