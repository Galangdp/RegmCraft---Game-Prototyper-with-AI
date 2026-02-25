import { Geist, Geist_Mono } from "next/font/google";
import "./globals.css";

const geistSans = Geist({
  variable: "--font-geist-sans",
  subsets: ["latin"],
});

const geistMono = Geist_Mono({
  variable: "--font-geist-mono",
  subsets: ["latin"],
});

export const metadata = {
  title: "RegmCraft | Next-Gen Game Prototyping Platform",
  description: "Professional web-based game development environment powered by C-Engine and Generative AI.",
  icons: {
    icon: "/favicon.svg",
    apple: "/apple-touch-icon.svg",
  },
};

import { Toaster } from "sonner";

export default function RootLayout({ children }) {
  return (
    <html lang="en" suppressHydrationWarning>
      <body
        className={`${geistSans.variable} ${geistMono.variable} antialiased`}
        suppressHydrationWarning
      >
        <Toaster position="top-right" theme="dark" richColors closeButton />
        <div id="root-layout-content">
          {children}
        </div>
      </body>
    </html>
  );
}
