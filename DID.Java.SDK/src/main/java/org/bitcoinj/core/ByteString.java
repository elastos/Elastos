
package org.bitcoinj.core;

import java.io.IOException;
import java.io.InputStream;
import java.io.InvalidObjectException;
import java.io.ObjectInputStream;
import java.io.OutputStream;
import java.io.Serializable;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.nio.charset.UnsupportedCharsetException;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.NoSuchElementException;

/**
 * Immutable sequence of bytes. Provides conversions to and from {@code byte[]}, {@link
 * java.lang.String}, {@link ByteBuffer}, {@link InputStream}, {@link OutputStream}.
 *
 * <p>Like {@link String}, the contents of a {@link ByteString} can never be observed to change, not
 * even in the presence of a data race or incorrect API usage in the client code.
 *
 * @author crazybob@google.com Bob Lee
 * @author kenton@google.com Kenton Varda
 * @author carlanton@google.com Carl Haverl
 * @author martinrb@google.com Martin Buchholz
 */
public abstract class ByteString implements Iterable<Byte>, Serializable {

  /**
   * When two strings to be concatenated have a combined length shorter than this, we just copy
   * their bytes on {@link #concat(ByteString)}. The trade-off is copy size versus the overhead of
   * creating tree nodes in {@link RopeByteString}.
   */
  static final int CONCATENATE_BY_COPY_SIZE = 128;

  /**
   * When copying an InputStream into a ByteString with .readFrom(), the chunks in the underlying
   * rope start at 256 bytes, but double each iteration up to 8192 bytes.
   */
  static final int MIN_READ_FROM_CHUNK_SIZE = 0x100; // 256b

  static final int MAX_READ_FROM_CHUNK_SIZE = 0x2000; // 8k

  public static final byte[] EMPTY_BYTE_ARRAY = new byte[0];

  /** Empty {@code ByteString}. */
  public static final ByteString EMPTY = new LiteralByteString(EMPTY_BYTE_ARRAY);

  /**
   * An interface to efficiently copy {@code byte[]}.
   *
   * <p>One of the noticeable costs of copying a byte[] into a new array using {@code
   * System.arraycopy} is nullification of a new buffer before the copy. It has been shown the
   * Hotspot VM is capable to intrisicfy {@code Arrays.copyOfRange} operation to avoid this
   * expensive nullification and provide substantial performance gain. Unfortunately this does not
   * hold on Android runtimes and could make the copy slightly slower due to additional code in the
   * {@code Arrays.copyOfRange}. Thus we provide two different implementation for array copier for
   * Hotspot and Android runtimes.
   */
  private interface ByteArrayCopier {
    /** Copies the specified range of the specified array into a new array */
    byte[] copyFrom(byte[] bytes, int offset, int size);
  }

  /** Implementation of {@code ByteArrayCopier} which uses {@link System#arraycopy}. */
  private static final class SystemByteArrayCopier implements ByteArrayCopier {
    @Override
    public byte[] copyFrom(byte[] bytes, int offset, int size) {
      byte[] copy = new byte[size];
      System.arraycopy(bytes, offset, copy, 0, size);
      return copy;
    }
  }

  /** Implementation of {@code ByteArrayCopier} which uses {@link Arrays#copyOfRange}. */
  private static final class ArraysByteArrayCopier implements ByteArrayCopier {
    @Override
    public byte[] copyFrom(byte[] bytes, int offset, int size) {
      return Arrays.copyOfRange(bytes, offset, offset + size);
    }
  }

  private static final ByteArrayCopier byteArrayCopier;

  static {
    byteArrayCopier = new ArraysByteArrayCopier();
  }

  /**
   * Cached hash value. Intentionally accessed via a data race, which is safe because of the Java
   * Memory Model's "no out-of-thin-air values" guarantees for ints. A value of 0 implies that the
   * hash has not been set.
   */
  private int hash = 0;

  // This constructor is here to prevent subclassing outside of this package,
  ByteString() {}

  /**
   * Gets the byte at the given index. This method should be used only for random access to
   * individual bytes. To access bytes sequentially, use the {@link ByteIterator} returned by {@link
   * #iterator()}, and call {@link #substring(int, int)} first if necessary.
   *
   * @param index index of byte
   * @return the value
   * @throws IndexOutOfBoundsException {@code index < 0 or index >= size}
   */
  public abstract byte byteAt(int index);

  /**
   * Gets the byte at the given index, assumes bounds checking has already been performed.
   *
   * @param index index of byte
   * @return the value
   * @throws IndexOutOfBoundsException {@code index < 0 or index >= size}
   */
  abstract byte internalByteAt(int index);

  /**
   * Return a {@link ByteString.ByteIterator} over the bytes in the ByteString. To avoid
   * auto-boxing, you may get the iterator manually and call {@link ByteIterator#nextByte()}.
   *
   * @return the iterator
   */
  @Override
  public ByteIterator iterator() {
    return new AbstractByteIterator() {
      private int position = 0;
      private final int limit = size();

      @Override
      public boolean hasNext() {
        return position < limit;
      }

      @Override
      public byte nextByte() {
        int currentPos = position;
        if (currentPos >= limit) {
          throw new NoSuchElementException();
        }
        position = currentPos + 1;
        return internalByteAt(currentPos);
      }
    };
  }

  /**
   * This interface extends {@code Iterator<Byte>}, so that we can return an unboxed {@code byte}.
   */
  public interface ByteIterator extends Iterator<Byte> {
    /**
     * An alternative to {@link Iterator#next()} that returns an unboxed primitive {@code byte}.
     *
     * @return the next {@code byte} in the iteration
     * @throws NoSuchElementException if the iteration has no more elements
     */
    byte nextByte();
  }

  abstract static class AbstractByteIterator implements ByteIterator {
    @Override
    public final Byte next() {
      // Boxing calls Byte.valueOf(byte), which does not instantiate.
      return nextByte();
    }

    @Override
    public final void remove() {
      throw new UnsupportedOperationException();
    }
  }

  /**
   * Gets the number of bytes.
   *
   * @return size in bytes
   */
  public abstract int size();

  /**
   * Returns {@code true} if the size is {@code 0}, {@code false} otherwise.
   *
   * @return true if this is zero bytes long
   */
  public final boolean isEmpty() {
    return size() == 0;
  }

  // =================================================================
  // Comparison

  private static final int UNSIGNED_BYTE_MASK = 0xFF;

  /**
   * Returns the value of the given byte as an integer, interpreting the byte as an unsigned value.
   * That is, returns {@code value + 256} if {@code value} is negative; {@code value} itself
   * otherwise.
   *
   * <p>Note: This code was copied from {@link com.google.common.primitives.UnsignedBytes#toInt}, as
   * Guava libraries cannot be used in the {@code com.google.protobuf} package.
   */
  private static int toInt(byte value) {
    return value & UNSIGNED_BYTE_MASK;
  }

  /**
   * Compares two {@link ByteString}s lexicographically, treating their contents as unsigned byte
   * values between 0 and 255 (inclusive).
   *
   * <p>For example, {@code (byte) -1} is considered to be greater than {@code (byte) 1} because it
   * is interpreted as an unsigned value, {@code 255}.
   */
  private static final Comparator<ByteString> UNSIGNED_LEXICOGRAPHICAL_COMPARATOR =
      new Comparator<ByteString>() {
        @Override
        public int compare(ByteString former, ByteString latter) {
          ByteIterator formerBytes = former.iterator();
          ByteIterator latterBytes = latter.iterator();

          while (formerBytes.hasNext() && latterBytes.hasNext()) {
            // Note: This code was copied from com.google.common.primitives.UnsignedBytes#compare,
            // as Guava libraries cannot be used in the {@code com.google.protobuf} package.
            int result =
                Integer.compare(toInt(formerBytes.nextByte()), toInt(latterBytes.nextByte()));
            if (result != 0) {
              return result;
            }
          }

          return Integer.compare(former.size(), latter.size());
        }
      };

  /**
   * Returns a {@link Comparator} which compares {@link ByteString}-s lexicographically
   * as sequences of unsigned bytes (i.e. values between 0 and 255, inclusive).
   *
   * <p>For example, {@code (byte) -1} is considered to be greater than {@code (byte) 1} because it
   * is interpreted as an unsigned value, {@code 255}:
   *
   * <ul>
   *   <li>{@code `-1` -> 0b11111111 (two's complement) -> 255}
   *   <li>{@code `1` -> 0b00000001 -> 1}
   * </ul>
   */
  public static Comparator<ByteString> unsignedLexicographicalComparator() {
    return UNSIGNED_LEXICOGRAPHICAL_COMPARATOR;
  }

  // =================================================================
  // ByteString -> substring

  /**
   * Return the substring from {@code beginIndex}, inclusive, to the end of the string.
   *
   * @param beginIndex start at this index
   * @return substring sharing underlying data
   * @throws IndexOutOfBoundsException if {@code beginIndex < 0} or {@code beginIndex > size()}.
   */
  public final ByteString substring(int beginIndex) {
    return substring(beginIndex, size());
  }

  /**
   * Return the substring from {@code beginIndex}, inclusive, to {@code endIndex}, exclusive.
   *
   * @param beginIndex start at this index
   * @param endIndex the last character is the one before this index
   * @return substring sharing underlying data
   * @throws IndexOutOfBoundsException if {@code beginIndex < 0}, {@code endIndex > size()}, or
   *     {@code beginIndex > endIndex}.
   */
  public abstract ByteString substring(int beginIndex, int endIndex);

  /**
   * Tests if this bytestring starts with the specified prefix. Similar to {@link
   * String#startsWith(String)}
   *
   * @param prefix the prefix.
   * @return <code>true</code> if the byte sequence represented by the argument is a prefix of the
   *     byte sequence represented by this string; <code>false</code> otherwise.
   */
  public final boolean startsWith(ByteString prefix) {
    return size() >= prefix.size() && substring(0, prefix.size()).equals(prefix);
  }

  /**
   * Tests if this bytestring ends with the specified suffix. Similar to {@link
   * String#endsWith(String)}
   *
   * @param suffix the suffix.
   * @return <code>true</code> if the byte sequence represented by the argument is a suffix of the
   *     byte sequence represented by this string; <code>false</code> otherwise.
   */
  public final boolean endsWith(ByteString suffix) {
    return size() >= suffix.size() && substring(size() - suffix.size()).equals(suffix);
  }

  // =================================================================
  // byte[] -> ByteString

  /**
   * Copies the given bytes into a {@code ByteString}.
   *
   * @param bytes source array
   * @param offset offset in source array
   * @param size number of bytes to copy
   * @return new {@code ByteString}
   * @throws IndexOutOfBoundsException if {@code offset} or {@code size} are out of bounds
   */
  public static ByteString copyFrom(byte[] bytes, int offset, int size) {
    checkRange(offset, offset + size, bytes.length);
    return new LiteralByteString(byteArrayCopier.copyFrom(bytes, offset, size));
  }

  /**
   * Copies the given bytes into a {@code ByteString}.
   *
   * @param bytes to copy
   * @return new {@code ByteString}
   */
  public static ByteString copyFrom(byte[] bytes) {
    return copyFrom(bytes, 0, bytes.length);
  }

  /**
   * Wraps the given bytes into a {@code ByteString}. Intended for internal only usage to force a
   * classload of ByteString before LiteralByteString.
   */
  static ByteString wrap(byte[] bytes) {
    // TODO(dweis): Return EMPTY when bytes are empty to reduce allocations?
    return new LiteralByteString(bytes);
  }

  /**
   * Wraps the given bytes into a {@code ByteString}. Intended for internal only usage to force a
   * classload of ByteString before BoundedByteString and LiteralByteString.
   */
  static ByteString wrap(byte[] bytes, int offset, int length) {
    return new BoundedByteString(bytes, offset, length);
  }

  /**
   * Copies the next {@code size} bytes from a {@code java.nio.ByteBuffer} into a {@code
   * ByteString}.
   *
   * @param bytes source buffer
   * @param size number of bytes to copy
   * @return new {@code ByteString}
   * @throws IndexOutOfBoundsException if {@code size > bytes.remaining()}
   */
  public static ByteString copyFrom(ByteBuffer bytes, int size) {
    checkRange(0, size, bytes.remaining());
    byte[] copy = new byte[size];
    bytes.get(copy);
    return new LiteralByteString(copy);
  }

  /**
   * Copies the remaining bytes from a {@code java.nio.ByteBuffer} into a {@code ByteString}.
   *
   * @param bytes sourceBuffer
   * @return new {@code ByteString}
   */
  public static ByteString copyFrom(ByteBuffer bytes) {
    return copyFrom(bytes, bytes.remaining());
  }

  /**
   * Encodes {@code text} into a sequence of bytes using the named charset and returns the result as
   * a {@code ByteString}.
   *
   * @param text source string
   * @param charsetName encoding to use
   * @return new {@code ByteString}
   * @throws UnsupportedEncodingException if the encoding isn't found
   */
  public static ByteString copyFrom(String text, String charsetName)
      throws UnsupportedEncodingException {
    return new LiteralByteString(text.getBytes(charsetName));
  }

  /**
   * Encodes {@code text} into a sequence of bytes using the named charset and returns the result as
   * a {@code ByteString}.
   *
   * @param text source string
   * @param charset encode using this charset
   * @return new {@code ByteString}
   */
  public static ByteString copyFrom(String text, Charset charset) {
    return new LiteralByteString(text.getBytes(charset));
  }

  /**
   * Blocks until a chunk of the given size can be made from the stream, or EOF is reached. Calls
   * read() repeatedly in case the given stream implementation doesn't completely fill the given
   * buffer in one read() call.
   *
   * @return A chunk of the desired size, or else a chunk as large as was available when end of
   *     stream was reached. Returns null if the given stream had no more data in it.
   */
  private static ByteString readChunk(InputStream in, final int chunkSize) throws IOException {
    final byte[] buf = new byte[chunkSize];
    int bytesRead = 0;
    while (bytesRead < chunkSize) {
      final int count = in.read(buf, bytesRead, chunkSize - bytesRead);
      if (count == -1) {
        break;
      }
      bytesRead += count;
    }

    if (bytesRead == 0) {
      return null;
    }

    // Always make a copy since InputStream could steal a reference to buf.
    return ByteString.copyFrom(buf, 0, bytesRead);
  }

  // =================================================================
  // ByteString -> byte[]

  /**
   * Copies bytes into a buffer at the given offset.
   *
   * <p>To copy a subset of bytes, you call this method on the return value of {@link
   * #substring(int, int)}. Example: {@code byteString.substring(start, end).copyTo(target, offset)}
   *
   * @param target buffer to copy into
   * @param offset in the target buffer
   * @throws IndexOutOfBoundsException if the offset is negative or too large
   */
  public void copyTo(byte[] target, int offset) {
    copyTo(target, 0, offset, size());
  }

  /**
   * Copies bytes into a buffer.
   *
   * @param target buffer to copy into
   * @param sourceOffset offset within these bytes
   * @param targetOffset offset within the target buffer
   * @param numberToCopy number of bytes to copy
   * @throws IndexOutOfBoundsException if an offset or size is negative or too large
   * @deprecated Instead, call {@code byteString.substring(sourceOffset, sourceOffset +
   *     numberToCopy).copyTo(target, targetOffset)}
   */
  @Deprecated
  public final void copyTo(byte[] target, int sourceOffset, int targetOffset, int numberToCopy) {
    checkRange(sourceOffset, sourceOffset + numberToCopy, size());
    checkRange(targetOffset, targetOffset + numberToCopy, target.length);
    if (numberToCopy > 0) {
      copyToInternal(target, sourceOffset, targetOffset, numberToCopy);
    }
  }

  /**
   * Internal (package private) implementation of {@link #copyTo(byte[],int,int,int)}. It assumes
   * that all error checking has already been performed and that {@code numberToCopy > 0}.
   */
  protected abstract void copyToInternal(
      byte[] target, int sourceOffset, int targetOffset, int numberToCopy);

  /**
   * Copies bytes into a ByteBuffer.
   *
   * <p>To copy a subset of bytes, you call this method on the return value of {@link
   * #substring(int, int)}. Example: {@code byteString.substring(start, end).copyTo(target)}
   *
   * @param target ByteBuffer to copy into.
   * @throws java.nio.ReadOnlyBufferException if the {@code target} is read-only
   * @throws java.nio.BufferOverflowException if the {@code target}'s remaining() space is not large
   *     enough to hold the data.
   */
  public abstract void copyTo(ByteBuffer target);

  /**
   * Copies bytes to a {@code byte[]}.
   *
   * @return copied bytes
   */
  public final byte[] toByteArray() {
    final int size = size();
    if (size == 0) {
      return EMPTY_BYTE_ARRAY;
    }
    byte[] result = new byte[size];
    copyToInternal(result, 0, 0, size);
    return result;
  }

  /**
   * Writes a copy of the contents of this byte string to the specified output stream argument.
   *
   * @param out the output stream to which to write the data.
   * @throws IOException if an I/O error occurs.
   */
  public abstract void writeTo(OutputStream out) throws IOException;

  /**
   * Writes a specified part of this byte string to an output stream.
   *
   * @param out the output stream to which to write the data.
   * @param sourceOffset offset within these bytes
   * @param numberToWrite number of bytes to write
   * @throws IOException if an I/O error occurs.
   * @throws IndexOutOfBoundsException if an offset or size is negative or too large
   */
  final void writeTo(OutputStream out, int sourceOffset, int numberToWrite) throws IOException {
    checkRange(sourceOffset, sourceOffset + numberToWrite, size());
    if (numberToWrite > 0) {
      writeToInternal(out, sourceOffset, numberToWrite);
    }
  }

  /**
   * Internal version of {@link #writeTo(OutputStream,int,int)} that assumes all error checking has
   * already been done.
   */
  abstract void writeToInternal(OutputStream out, int sourceOffset, int numberToWrite)
      throws IOException;

  /**
   * Constructs a read-only {@code java.nio.ByteBuffer} whose content is equal to the contents of
   * this byte string. The result uses the same backing array as the byte string, if possible.
   *
   * @return wrapped bytes
   */
  public abstract ByteBuffer asReadOnlyByteBuffer();

  /**
   * Constructs a list of read-only {@code java.nio.ByteBuffer} objects such that the concatenation
   * of their contents is equal to the contents of this byte string. The result uses the same
   * backing arrays as the byte string.
   *
   * <p>By returning a list, implementations of this method may be able to avoid copying even when
   * there are multiple backing arrays.
   *
   * @return a list of wrapped bytes
   */
  public abstract List<ByteBuffer> asReadOnlyByteBufferList();

  /**
   * Constructs a new {@code String} by decoding the bytes using the specified charset.
   *
   * @param charsetName encode using this charset
   * @return new string
   * @throws UnsupportedEncodingException if charset isn't recognized
   */
  public final String toString(String charsetName) throws UnsupportedEncodingException {
    try {
      return toString(Charset.forName(charsetName));
    } catch (UnsupportedCharsetException e) {
      UnsupportedEncodingException exception = new UnsupportedEncodingException(charsetName);
      exception.initCause(e);
      throw exception;
    }
  }

  /**
   * Constructs a new {@code String} by decoding the bytes using the specified charset. Returns the
   * same empty String if empty.
   *
   * @param charset encode using this charset
   * @return new string
   */
  public final String toString(Charset charset) {
    return size() == 0 ? "" : toStringInternal(charset);
  }

  /**
   * Constructs a new {@code String} by decoding the bytes using the specified charset.
   *
   * @param charset encode using this charset
   * @return new string
   */
  protected abstract String toStringInternal(Charset charset);

  // =================================================================
  // equals() and hashCode()

  @Override
  public abstract boolean equals(Object o);

  /** Base class for leaf {@link ByteString}s (i.e. non-ropes). */
  abstract static class LeafByteString extends ByteString {
    @Override
    protected final int getTreeDepth() {
      return 0;
    }

    @Override
    protected final boolean isBalanced() {
      return true;
    }

    /**
     * Check equality of the substring of given length of this object starting at zero with another
     * {@code ByteString} substring starting at offset.
     *
     * @param other what to compare a substring in
     * @param offset offset into other
     * @param length number of bytes to compare
     * @return true for equality of substrings, else false.
     */
    abstract boolean equalsRange(ByteString other, int offset, int length);
  }

  // =================================================================
  // Methods {@link RopeByteString} needs on instances, which aren't part of the
  // public API.

  /**
   * Return the depth of the tree representing this {@code ByteString}, if any, whose root is this
   * node. If this is a leaf node, return 0.
   *
   * @return tree depth or zero
   */
  protected abstract int getTreeDepth();

  /**
   * Return {@code true} if this ByteString is literal (a leaf node) or a flat-enough tree in the
   * sense of RopeByteString.
   *
   * @return true if the tree is flat enough
   */
  protected abstract boolean isBalanced();

  /**
   * Return the cached hash code if available.
   *
   * @return value of cached hash code or 0 if not computed yet
   */
  protected final int peekCachedHashCode() {
    return hash;
  }

  /**
   * Checks that the given index falls within the specified array size.
   *
   * @param index the index position to be tested
   * @param size the length of the array
   * @throws IndexOutOfBoundsException if the index does not fall within the array.
   */
  static void checkIndex(int index, int size) {
    if ((index | (size - (index + 1))) < 0) {
      if (index < 0) {
        throw new ArrayIndexOutOfBoundsException("Index < 0: " + index);
      }
      throw new ArrayIndexOutOfBoundsException("Index > length: " + index + ", " + size);
    }
  }

  /**
   * Checks that the given range falls within the bounds of an array
   *
   * @param startIndex the start index of the range (inclusive)
   * @param endIndex the end index of the range (exclusive)
   * @param size the size of the array.
   * @return the length of the range.
   * @throws IndexOutOfBoundsException some or all of the range falls outside of the array.
   */
  static int checkRange(int startIndex, int endIndex, int size) {
    final int length = endIndex - startIndex;
    if ((startIndex | endIndex | length | (size - endIndex)) < 0) {
      if (startIndex < 0) {
        throw new IndexOutOfBoundsException("Beginning index: " + startIndex + " < 0");
      }
      if (endIndex < startIndex) {
        throw new IndexOutOfBoundsException(
            "Beginning index larger than ending index: " + startIndex + ", " + endIndex);
      }
      // endIndex >= size
      throw new IndexOutOfBoundsException("End index: " + endIndex + " >= " + size);
    }
    return length;
  }

  /**
   * This class implements a com.google.protobuf.ByteString backed by a single array of
   * bytes, contiguous in memory. It supports substring by pointing to only a sub-range of the
   * underlying byte array, meaning that a substring will reference the full byte-array of the
   * string it's made from, exactly as with {@link String}.
   *
   * @author carlanton@google.com (Carl Haverl)
   */
  // Keep this class private to avoid deadlocks in classloading across threads as ByteString's
  // static initializer loads LiteralByteString and another thread loads LiteralByteString.
  private static class LiteralByteString extends ByteString.LeafByteString {
    private static final long serialVersionUID = 1L;

    protected final byte[] bytes;

    /**
     * Creates a {@code LiteralByteString} backed by the given array, without copying.
     *
     * @param bytes array to wrap
     */
    LiteralByteString(byte[] bytes) {
      if (bytes == null) {
        throw new NullPointerException();
      }
      this.bytes = bytes;
    }

    @Override
    public byte byteAt(int index) {
      // Unlike most methods in this class, this one is a direct implementation
      // ignoring the potential offset because we need to do range-checking in the
      // substring case anyway.
      return bytes[index];
    }

    @Override
    byte internalByteAt(int index) {
      return bytes[index];
    }

    @Override
    public int size() {
      return bytes.length;
    }

    // =================================================================
    // ByteString -> substring

    @Override
    public final ByteString substring(int beginIndex, int endIndex) {
      final int length = checkRange(beginIndex, endIndex, size());

      if (length == 0) {
        return ByteString.EMPTY;
      }

      return new BoundedByteString(bytes, getOffsetIntoBytes() + beginIndex, length);
    }

    // =================================================================
    // ByteString -> byte[]

    @Override
    protected void copyToInternal(
        byte[] target, int sourceOffset, int targetOffset, int numberToCopy) {
      // Optimized form, not for subclasses, since we don't call
      // getOffsetIntoBytes() or check the 'numberToCopy' parameter.
      // TODO(nathanmittler): Is not calling getOffsetIntoBytes really saving that much?
      System.arraycopy(bytes, sourceOffset, target, targetOffset, numberToCopy);
    }

    @Override
    public final void copyTo(ByteBuffer target) {
      target.put(bytes, getOffsetIntoBytes(), size()); // Copies bytes
    }

    @Override
    public final ByteBuffer asReadOnlyByteBuffer() {
      return ByteBuffer.wrap(bytes, getOffsetIntoBytes(), size()).asReadOnlyBuffer();
    }

    @Override
    public final List<ByteBuffer> asReadOnlyByteBufferList() {
      return Collections.singletonList(asReadOnlyByteBuffer());
    }

    @Override
    public final void writeTo(OutputStream outputStream) throws IOException {
      outputStream.write(toByteArray());
    }

    @Override
    final void writeToInternal(OutputStream outputStream, int sourceOffset, int numberToWrite)
        throws IOException {
      outputStream.write(bytes, getOffsetIntoBytes() + sourceOffset, numberToWrite);
    }

    @Override
    protected final String toStringInternal(Charset charset) {
      return new String(bytes, getOffsetIntoBytes(), size(), charset);
    }

    // =================================================================
    // equals() and hashCode()

    @Override
    public final boolean equals(Object other) {
      if (other == this) {
        return true;
      }
      if (!(other instanceof ByteString)) {
        return false;
      }

      if (size() != ((ByteString) other).size()) {
        return false;
      }
      if (size() == 0) {
        return true;
      }

      if (other instanceof LiteralByteString) {
        LiteralByteString otherAsLiteral = (LiteralByteString) other;
        // If we know the hash codes and they are not equal, we know the byte
        // strings are not equal.
        int thisHash = peekCachedHashCode();
        int thatHash = otherAsLiteral.peekCachedHashCode();
        if (thisHash != 0 && thatHash != 0 && thisHash != thatHash) {
          return false;
        }

        return equalsRange((LiteralByteString) other, 0, size());
      } else {
        // RopeByteString and NioByteString.
        return other.equals(this);
      }
    }

    /**
     * Check equality of the substring of given length of this object starting at zero with another
     * {@code LiteralByteString} substring starting at offset.
     *
     * @param other what to compare a substring in
     * @param offset offset into other
     * @param length number of bytes to compare
     * @return true for equality of substrings, else false.
     */
    @Override
    final boolean equalsRange(ByteString other, int offset, int length) {
      if (length > other.size()) {
        throw new IllegalArgumentException("Length too large: " + length + size());
      }
      if (offset + length > other.size()) {
        throw new IllegalArgumentException(
            "Ran off end of other: " + offset + ", " + length + ", " + other.size());
      }

      if (other instanceof LiteralByteString) {
        LiteralByteString lbsOther = (LiteralByteString) other;
        byte[] thisBytes = bytes;
        byte[] otherBytes = lbsOther.bytes;
        int thisLimit = getOffsetIntoBytes() + length;
        for (int thisIndex = getOffsetIntoBytes(),
                otherIndex = lbsOther.getOffsetIntoBytes() + offset;
            (thisIndex < thisLimit);
            ++thisIndex, ++otherIndex) {
          if (thisBytes[thisIndex] != otherBytes[otherIndex]) {
            return false;
          }
        }
        return true;
      }

      return other.substring(offset, offset + length).equals(substring(0, length));
    }


    // =================================================================
    // Internal methods

    /**
     * Offset into {@code bytes[]} to use, non-zero for substrings.
     *
     * @return always 0 for this class
     */
    protected int getOffsetIntoBytes() {
      return 0;
    }
  }

  /**
   * This class is used to represent the substring of a {@link ByteString} over a single byte array.
   * In terms of the public API of {@link ByteString}, you end up here by calling {@link
   * ByteString#copyFrom(byte[])} followed by {@link ByteString#substring(int, int)}.
   *
   * <p>This class contains most of the overhead involved in creating a substring from a {@link
   * LiteralByteString}. The overhead involves some range-checking and two extra fields.
   *
   * @author carlanton@google.com (Carl Haverl)
   */
  // Keep this class private to avoid deadlocks in classloading across threads as ByteString's
  // static initializer loads LiteralByteString and another thread loads BoundedByteString.
  private static final class BoundedByteString extends LiteralByteString {

    private final int bytesOffset;
    private final int bytesLength;

    /**
     * Creates a {@code BoundedByteString} backed by the sub-range of given array, without copying.
     *
     * @param bytes array to wrap
     * @param offset index to first byte to use in bytes
     * @param length number of bytes to use from bytes
     * @throws IllegalArgumentException if {@code offset < 0}, {@code length < 0}, or if {@code
     *     offset + length > bytes.length}.
     */
    BoundedByteString(byte[] bytes, int offset, int length) {
      super(bytes);
      checkRange(offset, offset + length, bytes.length);

      this.bytesOffset = offset;
      this.bytesLength = length;
    }

    /**
     * Gets the byte at the given index. Throws {@link ArrayIndexOutOfBoundsException} for
     * backwards-compatibility reasons although it would more properly be {@link
     * IndexOutOfBoundsException}.
     *
     * @param index index of byte
     * @return the value
     * @throws ArrayIndexOutOfBoundsException {@code index} is < 0 or >= size
     */
    @Override
    public byte byteAt(int index) {
      // We must check the index ourselves as we cannot rely on Java array index
      // checking for substrings.
      checkIndex(index, size());
      return bytes[bytesOffset + index];
    }

    @Override
    byte internalByteAt(int index) {
      return bytes[bytesOffset + index];
    }

    @Override
    public int size() {
      return bytesLength;
    }

    @Override
    protected int getOffsetIntoBytes() {
      return bytesOffset;
    }

    // =================================================================
    // ByteString -> byte[]

    @Override
    protected void copyToInternal(
        byte[] target, int sourceOffset, int targetOffset, int numberToCopy) {
      System.arraycopy(
          bytes, getOffsetIntoBytes() + sourceOffset, target, targetOffset, numberToCopy);
    }

    // =================================================================
    // Serializable

    private static final long serialVersionUID = 1L;

    Object writeReplace() {
      return ByteString.wrap(toByteArray());
    }

    private void readObject(@SuppressWarnings("unused") ObjectInputStream in) throws IOException {
      throw new InvalidObjectException(
          "BoundedByteStream instances are not to be serialized directly");
    }
  }
}
