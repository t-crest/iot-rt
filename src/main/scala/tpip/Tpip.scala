package tpip

/**
 * This is the main class for the tpIP stack
 */
class Tpip(host: String) extends Runnable {

  // Just hard code BlaaHund link layer
  val ll: LinkLayer = new BlaaHund(host)

  // Needs to be called periodically to keep things going
  def run() = {
    ll.run()
  }
}