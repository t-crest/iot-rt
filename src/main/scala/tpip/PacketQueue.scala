package tpip

/**
 * Single reader, single writer non-blocking queue
 */
class PacketQueue(capacity: Int) {

  val data = new Array[Packet](capacity+1)
	@volatile var rdPtr = 0
	@volatile var wrPtr = 0

	def empty() = rdPtr==wrPtr
	
	def full() = {
		var i = wrPtr+1
		// >= makes Wolfgang's DFA happy
		if (i>=data.length) i=0
		i==rdPtr
	}

	def enq(p: Packet) = {
		var i = wrPtr
		data(i) = p
		i += 1
		if (i>=data.length) i=0
		wrPtr = i
	}

	/**
	 * Dequeue of an element. Returns null for an empty queue.
	 * @return the value or null
	 */
	def deq(): Packet = {
		if (rdPtr==wrPtr) return null
		var i =rdPtr
		val p = data(i)
		i += 1
		if (i>=data.length) i=0
		rdPtr = i
		p
	}
	
	def cnt() = {
		var i = wrPtr-rdPtr
		if (i<0) i+=data.length
		i
	}
	
	def free() = data.length-1-cnt
	
	def checkedEnq(p: Packet): Boolean = {
		if (full()) return false;
		enq(p);
		return true;
	}

}