using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace ChangeCharset
{
    class Program
    {
        static void Main(string[] args)
        {
            
            Encoding shift_jis;
            shift_jis=Encoding.GetEncoding(932);
            foreach (var f in new DirectoryInfo(@"D:\Program Files\Alwil Software\Adobe\bcdd\XQL\tsksrc\Test").GetFiles("*.txt", SearchOption.TopDirectoryOnly))
            {
                string s = File.ReadAllText(f.FullName, Encoding.UTF8);
                File.WriteAllText(f.FullName, s, shift_jis);
            }
        }
    }
}
