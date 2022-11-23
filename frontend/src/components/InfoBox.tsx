import { motion } from 'framer-motion';
import React from 'react';

import '../styles/InfoBox.css';

interface InfoBoxProps {
  title: string;
  color: string;
  children: React.ReactNode | null;
}

export default function InfoBox({ title, children, color }: InfoBoxProps) {
  return (
    <motion.div style={{ background: color }} className="InfoBox">
      <h2>{title}</h2>
      {children}
    </motion.div>
  );
}
