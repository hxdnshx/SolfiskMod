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
            
            Encoding shift_js;
            shift_js=Encoding.GetEncoding(932);
            foreach (var f in new DirectoryInfo(@"D:\Program Files\Alwil Software\Adobe\bcdd\XQL\report\src").GetFiles("*.rb", SearchOption.TopDirectoryOnly))
            {
                string s = File.ReadAllText(f.FullName,shift_js);
                File.WriteAllText(f.FullName, s, Encoding.UTF8);
            }
        }
    }
}
