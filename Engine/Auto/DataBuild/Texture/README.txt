- 两个文件的功能是一样的
- 默认认为：
  1. 文件名带有 baseColor 字符串的是非线性的 ambient 贴图
  2. 文件名带有 normal    字符串的是线性的法线贴图
  3. 文件名带有 roughness 字符串的是线性的合并了多个通道的贴图
  4. 更多的贴图类型需要手动定义一下
- 写的比较简陋，请务必打开文件确认一下贴图类型、命名方式和路径是否符合预期

使用示例：
- textures（存放贴图源文件的文件夹）
-- baseColor.png
- compiled（存放编码结果的文件夹）
- texture.bat
- texturec.exe
