export const Did = {
  did: String,
  expirationDate: Date,
  number: {
    type: String, // uuid string
    required: true,
    unique: true
  },
  message: String,
  success: Boolean
}
