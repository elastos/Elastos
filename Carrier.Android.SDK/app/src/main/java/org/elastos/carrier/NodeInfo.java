package org.elastos.carrier;

/**
 * A class representing the Carrier node information.
 *
 * In Carrier SDK, all nodes have same node attributes.
 */
public class NodeInfo {

	/**
	 * Carrier node ID max length.
	 */
	public static final int MAX_ID_LEN = 63;

	/**
	 * Carrier node name max length.
	 */
	public static final int MAX_NODE_NAME_LEN = 63;

	/**
	 * Carrier node description max length.
	 */
	public static final int MAX_NODE_DESCRIPTION_LEN = 127;

	private String nodeId;
	private String name;
	private String description;

	/**
	 * Set the node ID.
	 *
	 * This function only be called in java-JNI.
	 *
	 * @param
	 *   	nodeId			The node ID.
	 */
	void setNodeId(String nodeId) {
		this.nodeId = nodeId;
	}

	/**
	 * Get the node ID
	 *
	 * @return
	 * 		The node ID.
     */
	public String getNodeId() {
		return nodeId;
	}

	/**
	 * Set the node name
	 *
	 * @param
	 * 		name			The new node name.
	 *
	 * @throws
	 * 		IllegalArgumentException
     */
	public void setName(String name) {
		if (name == null || name.length() > MAX_NODE_NAME_LEN)
			throw new IllegalArgumentException("Invalid name length, expected (0," +
				MAX_NODE_NAME_LEN + "]");
		this.name = name;
	}

	/**
	 * Get the node name.
	 *
	 * @return
	 * 		The node name
     */
	public String getName() {
		return name;
	}

	/**
	 * Set the node's description.
	 *
	 * @param
	 * 		description		The new description
	 *
	 * @throws
	 * 		IllegalArgumentException
	 */
	public void setDescription(String description) {
		if (description == null || description.length() > MAX_NODE_DESCRIPTION_LEN)
			throw new IllegalArgumentException("Invalid description length, expected (0," +
				MAX_NODE_DESCRIPTION_LEN + "]");
		this.description = description;
	}

	/**
	 * Get the node's description.
	 *
	 * @return
	 * 		The node's description
     */
	public String getDescription() {
		return description;
	}

	/**
	 * Get debug decription of current NodeInfo object.
	 *
	 * @return
	 * 		The debug description of current NodeInfo object.
     */
	@Override
	public String toString() {
		return String.format("NodeInfo[id: %s, name:%s, description:%s]", nodeId, name,
				description);
	}
}
