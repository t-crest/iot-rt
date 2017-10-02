package tpip

object Logger {
  
  var ll: BlaaHund = null
  def setLL(bh: BlaaHund) {
    ll = bh
  }
  def log(s: String) {
    println(s)
    ll.log(s)
  }
}