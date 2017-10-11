package tpip

class Packet {

  val MAX_LEN = 1000

  val buf = new Array[Byte](MAX_LEN)
  var len = 0

  def set(b: Array[Byte]): Unit = {

    len = if (b.length > buf.length) buf.length else b.length

    for (i <- 0 until len) {
      buf(i) = b(i)
    }
  }
  
  def copy(p: Packet): Unit = {
    for (i <- 0 until p.len) {
      buf(i) = p.buf(i)
    }
    len = p.len
  }

  def setWord(pos: Int, word: Int): Unit = {
    buf(pos) = (word >>> 24).toByte
    buf(pos + 1) = (word >>> 16).toByte
    buf(pos + 2) = (word >>> 8).toByte
    buf(pos + 3) = word.toByte
  }

  def setHalfWord(pos: Int, hw: Int): Unit = {
    buf(pos) = (hw >>> 8).toByte
    buf(pos + 1) = hw.toByte
  }

  def getWord(pos: Int): Int = {
    ((buf(pos).toInt & 0xff) << 24) +
      ((buf(pos + 1).toInt & 0xff) << 16) +
      ((buf(pos + 2).toInt & 0xff) << 8) +
      ((buf(pos + 3).toInt & 0xff))
  }

  def getHalfWord(pos: Int): Int = {
    (((buf(pos).toInt & 0xff) << 8) +
      ((buf(pos + 1).toInt & 0xff))) & 0xffff
  }
  
  def getSource = getWord(12)
  def getDest = getWord(16)
  def setSource(n: Int) = setWord(12, n)
  def setDest(n: Int) = setWord(16, n)

  def checkSum(off: Int, cnt: Int): Int = {

    if ((cnt & 0x01) != 0) {
      buf(cnt) = 0
    }
    var sum = 0
    for (i <- 0 until cnt / 2) {
      sum += ((buf(i * 2) << 8) + buf(i * 2 + 1) & 0xff) & 0xffff
    }
    while ((sum >> 16) != 0)
      sum = (sum & 0xffff) + (sum >> 16)

    (~sum) & 0xffff
  }
  
//  overrides def toString: String = {
//    var s = "Packet:\n"
//    s
//  }
}