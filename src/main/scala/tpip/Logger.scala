package tpip

object Logger {
  
  var ll: BlaaHund = null
  def setLL(bh: BlaaHund) {
    ll = bh
  }
  def log(s: String) {
    println("log: "+s)
    ll.log(s)
  }
}