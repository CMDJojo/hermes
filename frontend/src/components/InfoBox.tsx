import { motion } from 'framer-motion';
import React from 'react';

import '../styles/InfoBox.css';

interface InfoBoxProps {
  title: string;
  color: string;
  textColor?: string;
  children: React.ReactNode | null;
}

export default function InfoBox({
  title,
  children,
  color,
  textColor,
}: InfoBoxProps) {
  return (
    <motion.div
      style={{ background: color, color: textColor ?? 'white' }}
      className="InfoBox"
    >
      <h2>{title}</h2>
      {children}
    </motion.div>
  );
}
