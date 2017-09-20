
package iotrt

import java.net._
import java.io._
import scala.io._

/**
 * We use a virtual link layer by sending an IP packet via
 * a HTTP GET request. The packet has a 2 byte header encoding
 * the length. The packet itself is encoded in hexadecimal
 * and forms the URL for the GET request.
 *
 * Connection is closed after each GET request.
 *
 * We call this virtual link layer Blaa Hund.
 */

class BlaaHundServer {

  def run(): Unit = {

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
      println(msg)

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

class BlaaHundClient(host: String) {

  val correctGet = "GET /index.html HTTP/1.1\r\n" +
    "Host: www.example.com\r\n\r\n"
  val http10Get = "GET /index.html HTTP/1.0\r\n\r\n"

  def run(): Unit = {
    val inetAddress = InetAddress.getByName(host)
    println(inetAddress)
    val s = new Socket(host, 80)
    val in = new BufferedSource(s.getInputStream())
    val out = new PrintStream(s.getOutputStream())
    out.print("GET /abcde HTTP/1.1\r\nHost: "+host+"\r\n\r\n")
    in.getLines().foreach(println)
    s.close()
  }
}

object BlaaHundClient {
  def main(args: Array[String]) {
    val c = new BlaaHundClient(args(0))
    c.run()
  }
}

object BlaaHundServer {

  def main(args: Array[String]) {
    val s = new BlaaHundServer()
    s.run()
  }
}

  /*
 * a complete response would be:
 
  val resp = "HTTP/1.1 200 OK\r\n\r\n" +
    "Content-Type: text/plain\r\n" +
    "Content-Length: 12\r\n" +
    "Connection: close\r\n\r\n" +

 */