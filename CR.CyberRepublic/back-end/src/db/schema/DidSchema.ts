export const Did = {
  did: String,
  number: {
    type: String, // uuid string
    required: true,
    unique: true
  },
  message: String,
  success: Boolean
}
