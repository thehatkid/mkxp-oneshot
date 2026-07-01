def embed(input, name = nil)
  basename = File.basename(input).gsub(/[^a-zA-Z0-9]/, '_')
  if name && !name.empty?
    prefix = name.gsub(/[^a-zA-Z0-9]/, '_')
    basename = "#{prefix}_#{basename}"
  end
  var = "mkxp_#{basename}"
  varlen = "mkxp_#{basename}_len"
  defname = "MKXP_EMBED_#{basename.upcase}_XXD"

  bytes = File.binread(input).bytes
  bytes_fmt = bytes.each_slice(12).map do |slice|
    "\t" + slice.map { |byte| "0x%02x" % byte }.join(", ")
  end.join(",\n")

  content = <<~SRC
    #ifndef #{defname}
    #define #{defname}

    #include <stddef.h>
    #include <stdint.h>

    #ifdef __cplusplus
    extern
    #endif
    const uint8_t #{var}[] = {
    #{bytes_fmt}
    };

    #ifdef __cplusplus
    extern
    #endif
    const size_t #{varlen} = #{bytes.size}ULL;

    #endif // #{defname}
  SRC

  return content
end

arg0 = File.basename(__FILE__)

if ARGV.empty?
  STDERR.puts("#{arg0}: usage: #{arg0} <input> [output] [name]")
  exit(1)
end

input = ARGV[0]
output = ARGV[1]
name = ARGV[2]

unless File.exist?(input)
  STDERR.puts("#{arg0}: #{input}: No such file")
  exit(1)
end

result = embed(input, name)
if output && !output.empty?
  File.write(output, result)
else
  STDOUT.puts(result)
end
