package iotrt

/**
 * A simple example app for the TCP/IP stack
 */
object App extends App {

  println("Hello from the real-time TCP/IP app")
  
  val server = new BlaaHundServer()
  
  // This is our simulated periodic loop.
  // It is not correct as we wait a constant time and not for the next period.
  while (true) {
    println("ping")
    Thread.sleep(100)
//    server.run()
  }
}
