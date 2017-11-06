
package tpip

import java.net._
import java.io._
import scala.io._

/**
 * We use a virtual link layer by sending an IP packet via
 * a HTTP GET request. The packet itself is encoded in hexadecimal
 * and forms the URL for the GET request.
 *
 * Connection is closed after each GET request.
 *
 * We call this virtual link layer Blaa Hund.
 */

class BlaaHund() extends LinkLayer {

  val correctGet = "GET /index.html HTTP/1.1\r\n" +
    "Host: www.example.com\r\n\r\n"
  val http10Get = "GET /index.html HTTP/1.0\r\n\r\n"
  
  val ipRasmus = 0x0a000001
  val ipMartin = 0x0a000002
  
  val hostRasmus = "pingrt.ngrok.io"
  val hostMartin = "iprt.ngrok.io"

  println("Hello I am a simple Blaa Hund server!")
  val server = new ServerSocket(8080)
  
  var logMsg = ""
  def log(s: String): Unit = {
    logMsg = s + logMsg + "\r\n"
  }
  
  Logger.setLL(this)

  // why can't we simple pass server into a new Thread?
  new Thread(new Runnable {
    def run() {
      serve()
    }
  }).start

  // this should be called from the TCP/IP stack infrastructure periodically
  // contains the client code
  def run(): Unit = {

    if (!txChannel.queue.empty) {
      println("Client got a packet to send")
      val p = txChannel.queue.deq()
      println(p)
      println("dest: " + p.getDest)
      printf("dest: 0x%08x\n", p.getDest)
      println("rasmus:" + ipRasmus)
      println("martin:" + ipMartin)
      val blaaPacket = Util.toHex(p.buf, p.len)
      println("BlaaHund: "+blaaPacket)
      txChannel.freePool.enq(p)
      
      // Super dummy routing
      val lhost = if (p.getDest == ipRasmus || p.getDest == 0x10000001) hostRasmus else hostMartin
      println("Sending to: " + lhost)
      
      val inetAddress = InetAddress.getByName(lhost)
      println(inetAddress)
      val s = new Socket(lhost, 80)
      val in = new BufferedSource(s.getInputStream())
      val out = new PrintStream(s.getOutputStream())
      val requ = "GET /" + blaaPacket + " HTTP/1.1\r\nHost: " + lhost + "\r\n\r\n"
      println(requ)
      out.print(requ)
      // in.getLines().foreach(println)
      val it = in.getLines()
      println(it.next())
      println(it.next())
      println(it.next())
      println(it.next())
      println(it.next())
      s.close()
    }
  }

  // This shall run in it's own thread as it is blocking on accept
  def serve(): Unit = {

    while (true) {

      val s = server.accept()
      println("Accepted connection")

      val in = new BufferedSource(s.getInputStream()).getLines()
      val out = new PrintStream(s.getOutputStream())

      val blaa = in.next.split(" ")(1).substring(1).toLowerCase()

      val p = rxChannel.freePool.deq()
      if (p != null) {
        p.set(Util.toBytes(blaa))
        rxChannel.queue.enq(p)
      } else {
        println("No free buffers, packet dropped")
      }
      println("rxChannel (in Blaa): "+ rxChannel.queue.cnt())

      logMsg = ""
      val okString = (blaa.filter(Util.isHexDec)).length == blaa.length
      val msg = if (okString) {
        "You sent me a Blaa Hund package: " + blaa + "\r\n"
      } else {
        "Your package is not a Blaa Hund package: " + blaa + "\r\n"
      }
      println(msg)
      Thread.sleep(1000)
      

      val resp = "HTTP/1.0 200 OK\r\n" +
        "Content-type: text/plain; charset=UTF-8\r\n\r\n" +
        "Hello World in plain text\r\n" + msg + logMsg
        
//        "<html><head></head><body><h2>Hello Real-Time IoT World!</h2>" +
//        msg +
//        "</body></html>\r\n\r\n"

      out.print(resp)
      out.flush()
      println("Close connection");
      s.close()
    }

  }

}

object Util {

  def isHexDec(c: Char) = {
    (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
  }

  def toHex(buffer: Array[Byte], len: Int): String = {

    def oneByte(b: Int): String = {
      def hex(b: Int): Char = if (b >= 0 && b <= 9) (b + '0').toChar else (b - 10 + 'a').toChar

      hex((b >> 4) & 0x0f).toString + hex(b & 0x0f).toString
    }

    var s = ""
    for (i <- 0 until len) {
      s = s + oneByte(buffer(i))
    }
    println(s)
    s
  }

  def toBytes(s: String): Array[Byte] = {

    val buf = new Array[Byte](s.length / 2)

    assert((s.filter(Util.isHexDec)).length == s.length)

    def oneByte(ch: Char, cl: Char): Byte = {

      def fromHex(c: Char): Int = if (c >= '0' && c <= '9') c - '0' else c - 'a' + 10

      (fromHex(ch) * 16 + fromHex(cl)).toByte
    }

    for (i <- 0 until s.length / 2) {
      buf(i) = oneByte(s(i * 2), s(i * 2 + 1))
    }

    buf
  }
}

  /*
 * a complete response would be:
 
  val resp = "HTTP/1.1 200 OK\r\n\r\n" +
    "Content-Type: text/plain\r\n" +
    "Content-Length: 12\r\n" +
    "Connection: close\r\n\r\n" +

 */
