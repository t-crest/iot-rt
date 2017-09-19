/**
 * A super simple web server.
 */
package iotrt

import java.net._
import java.io._
import scala.io._

object BlaaHund {

  def main(args: Array[String]) {

    println("Hello I am a simple RT IoT server!")
    val server = new ServerSocket(8080)
    while (true) {
      val s = server.accept()
      println("Accepted connection")

      val in = new BufferedSource(s.getInputStream()).getLines()
      val out = new PrintStream(s.getOutputStream())

      val blaa = in.next.split(" ")(1).substring(1).toLowerCase()

      def isHexDec(c: Char) = {
        (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
      }

      val okString = (blaa.filter(isHexDec)).length == blaa.length
      val msg = if (okString) {
        "You sent me a Blaa Hund package: " + blaa + "\n"
      } else {
        "Your package is not a Blaa Hund package: " + blaa + "\n"
      }

      val resp = "HTTP/1.0 200 OK\r\n\r\n" +
        "<html><head></head><body><h2>Hello Real-Time IoT World!</h2>" +
        msg +
        "</body></html>\r\n\r\n"

      out.print(resp)
      out.flush()
      println("Close connection");
      s.close()
    }
  }
}

  /*
 * a complete response would be:
 
  val resp = "HTTP/1.1 200 OK\r\n\r\n" +
    "Content-Type: text/plain\r\n" +
    "Content-Length: 12\r\n" +
    "Connection: close\r\n\r\n" +

 */