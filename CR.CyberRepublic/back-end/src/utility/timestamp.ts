const timestamp = {
    millisecond (date: Date) {
        return new Date(date).getTime()
    },
    second (date: Date) {
        return Math.trunc(this.millisecond(date) / 1000)
    }
}

export default timestamp