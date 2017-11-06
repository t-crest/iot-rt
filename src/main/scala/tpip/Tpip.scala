package tpip

/**
 * This is the main class for the tpIP stack
 */
class Tpip() extends Runnable {

  // Just hard code BlaaHund link layer
  val ll: LinkLayer = new BlaaHund()
  val ip = new Ip()

  val PROT_ICMP = 1
  val ICMP_ECHO_REQUEST = 8
  val ICMP_ECHO_REPLY = 0

  // Needs to be called periodically to keep things going
  def run() = {
    ll.run()

    if (!ll.rxChannel.queue.empty()) {
      val pr = receive(ll.rxChannel.queue.deq())
      // just do ICMP ping reply without looking at the packet at all

      if (pr != null) {
        val prot = pr.getByte(9)
        if (prot == PROT_ICMP) {
          if (pr.getByte(20) == ICMP_ECHO_REQUEST) {
            Logger.log("ICMP echo request: do the ICMP reply.")

            if (!ll.txChannel.freePool.empty()) {
              val ps = ll.txChannel.freePool.deq()
              ps.copy(pr)
              ps.setByte(20, ICMP_ECHO_REPLY)
              ip.doIp(ps, PROT_ICMP, pr.getDest, pr.getSource)
              ll.txChannel.queue.enq(ps)
            }
          } else {
            Logger.log("Just got a ICMP echo reply. Nothing to do.")
          }
        } else {
          Logger.log("Not yet handled protocol")
        }
      }
      ll.rxChannel.freePool.enq(pr)
    }
  }

  def receive(p: Packet): Packet = {
    val buf = p.buf

    val len = p.getHalfWord(2)
    if (len > p.len || buf(0) != 0x45) {
      Logger.log("len " + len + " " + buf(0))
      Logger.log("Too long or IP options -> drop it")
      ll.rxChannel.freePool.enq(p)
      return null
    }

    // todo CheckSum
//    val prot = buf(9)
//    if (prot == PROT_ICMP) {
//      val typ = buf(20)
//      val code = buf(21)
//      if (typ == 8) {
//        Logger.log("Got a ping")
//        buf(20) = 0
//      } else {
//        Logger.log("No ping -> I drop you")
//        ll.rxChannel.freePool.enq(p)
//        return null
//      }
//    } else {
//      Logger.log("I do not understand you -> I drop you")
//      ll.rxChannel.freePool.enq(p)
//      return null
//    }
//    println(len)
    p
  }

  /**
   * Process one IP packet. Change buffer and set length to get a packet sent
   * back. called from Net.loop().
   */
  //	public void receive(Packet p) {
  //
  //		int i;
  //		int[] buf = p.buf;
  //		int len;
  //
  //		i = buf[0];
  //		len = i & 0xffff; // length from IP header
  //		// NO options are assumed in ICMP/TCP/IP...
  //		// => copy if options present
  //		// but we just drop it now - too lazy
  //		if (len > p.len || (i >>> 24 != 0x45)) {
  //			if (Logging.LOG) Logging.wr("IP options -> discard");
  //			ejip.returnPacket(p); // packet to short or ip options => drop it
  //			return;
  //		} else {
  //			p.len = len; // correct for to long packets
  //		}
  //
  //		// TODO fragmentation
  //		if (Ip.chkSum(buf, 0, 20) != 0) {
  //			ejip.returnPacket(p);
  //			if (Logging.LOG) Logging.wr("wrong IP checksum ");
  //			return;
  //		}
  //
  //		int prot = (buf[2] >> 16) & 0xff; // protocol
  //		if (prot == PROT_ICMP) {
  //			doICMP(p);
  //			ip.doIp(p, prot);
  //		} else if (prot == Tcp.PROTOCOL) {
  //			if (Ejip.TCP_ENABLED) {
  //				// that's the new TCP processing
  //				tcp.process(p);					
  //			} else {
  //				ejip.returnPacket(p); // mark packet free					
  //			}
  //		} else if (prot == Udp.PROTOCOL) {
  //			udp.process(p); // Udp generates the reply
  //		} else {
  //			ejip.returnPacket(p); // mark packet free
  //		}
  //	}

  //	/**
  //	 * the famous ping.
  //	 */
  //	private void doICMP(Packet p) {
  //
  //		int type_code = p.buf[5] >>> 16;
  //		if (Logging.LOG) Logging.wr('P');
  //		if (Logging.LOG) Logging.hexVal(type_code);
  //		if (type_code == 0x0800) {
  //			// TODO check received ICMP checksum
  //			p.buf[5] = 0; // echo replay plus clear checksum,
  //			p.buf[5] = Ip.chkSum(p.buf, 5, p.len - 20); // echo replay (0x0000)
  //														// plus checksum
  //		} else {
  //			p.len = 0;
  //		}
  //	}

}
